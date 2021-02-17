# Judge

根据判题数据，判定用户程序的运行结果以及获取用户程序运行时间和内存消耗。

判题结果会以 JSON 格式输出到标准输出中。

结果：

```json
{
  "status": 0,
  "cpu_time_used": 0,
  "cpu_time_used_us": 479,
  "real_time_used": 2,
  "real_time_used_us": 1966,
  "memory_used": 1556,
  "signal": 0,
  "exit_code": 0
}
```

## 编译

```bash
make judge
```

会在当前目录编译出一个可执行文件 `judge`。

```bash
make libjudge
```

会在当前目录编译出一个共享库 `libjudge.so`。

## 运行

```plain
❯ ./judge

Usage: judge <command> [<args>]

Available commands are:

judge   Run then compare.
run     Run the specified command only, do not check the result.
check   Compare the user's output and right answer to get the result.

Type 'judge help <command>' to get help for a specific command.
```

程序有三种命令模式：

- judge
  完整判题模式
- run
  只根据输入运行用户程序
- check
  给入题目答案数据和用户输出的数据，输出一个判题结果。

### judge 模式

```plain
❯ ./judge help judge

Usage: judge judge <command> <time_limit> <memory_limit> <testdata_input_path> <testdata_output_path> <tmp_output_path> [options]

e.g. judge process with input data file and tmp output path, and log path.
        ./judge judge ./main 1000 2048 ./tests/1/1.in ./tests/1/1.out 1.tmp.out -l 1.log

Options:

  -l    Path of the log file
```

- command 用户程序地址，如果命令有空格，需要给命令加上双引号，如：`"node main.js"`
- time_limit 时间单位是 ms
- memory_limit 内存单位是 kb
- input_path 判题的标准输入文件位置
- output_path 判题的标准输出文件位置
- tmp_output_path 用户程序执行的标准输出位置（用于判断答案是否正确）
- -l 参数可以传入一个 log 文件的地址，会把判题 log 写入该文件，方便调试。

之所以要多传入一个 `<tmp_output_path>`(`1.tmp.out`) 是因为可以：

1. 方便多步对程序执行结果进行判断。
2. 不用把判题输出保留在内存中。

### run 模式

```plain
❯ ./judge help run

Usage: judge run <command> <time_limit> <memory_limit> <testdata_input_path> <tmp_output_path> [options]

e.g. Run process with input data file and tmp output path, and log path.
        ./judge run ./main 1000 2048 ./tests/1/1.in 1.tmp.out -l 1.log

Options:

  -l    Path of the log file
```

各字段意义见 [judge 模式](#judge-模式)。

### check 模式

```plain
❯ ./judge help check

Usage: judge check <testdata_output_path> <tmp_output_path> [options]

e.g. Judge answers with <testdata_output_path> and <tmp_output_path>.
        ./judge check ./tests/1/1.out 1.tmp.out -l 1.log

Options:

  -l    Path of the log file%
```

各字段意义见 [judge 模式](#judge-模式)。

## 运行结果

首先要判断 judge 程序是否运行成功，看进程的退出值。

这个退出值是判题程序判题是否成功的标识。

然后以 JSON 形式读取判题程序的标准输出（JSON 格式），看以下两个值：

```json
  "signal": 0,
  "exit_code": 0
```

`signal` 是导致子程序退出的信号值。
`exit_code` 是子程序的退出值。

如果都为 0，则说明本次判题执行成功。
如果有不为 0 的值，可以在判题日志中查看更多信息。

status 是判题结果：

```c
#define PENDING -1 // 还未执行答案检查
#define ACCEPTED 0
#define PRESENTATION_ERROR 1
#define TIME_LIMIT_EXCEEDED 2
#define MEMORY_LIMIT_EXCEEDED 3
#define WRONG_ANSWER 4
#define RUNTIME_ERROR 5
#define OUTPUT_LIMIT_EXCEEDED 6
#define COMPILE_ERROR 7
#define SYSTEM_ERROR 8
```

注意：如果仅执行了 `run` 模式，并且程序运行没有错误或者超过资源限制的话，输出结果的 `status` 应该为: `-1`。

### 输出单位

其中 `cpu_time_used` 和 `real_time_used` 单位都是毫秒(ms)。
`cpu_time_used_us` 和 `real_time_used_us` 单位是微秒(us)。

`cpu_time` 的意思是用户在程序中用到的 CPU 计算所消耗的时间，不包括 IO 或者挂起时间。
`real_time` 是用户程序真实运行的时间。

`memory_used` 在 linux 下单位是 kb。

### check 模式输出

如果运行 `check` 模式的话，只会输出一个 int 值代表判题结果，如：

```bash
❯ ./judge check -l 1.log ./tests/1/1.out 1.tmp.out
0
```

说明答案正确，判题状态是 ACCEPT。

## FAQ

### rusage 的 `ru_utime` 和 `ru_stime` 有什么区别？

[来源](https://www.reddit.com/r/cs50/comments/553okd/difference_between_ru_utime_and_ru_stime/)

操作系统出现的原因就是为了在同时运行多个程序时可以共享对硬件的访问。

CPU 时间有时候花费是在运行用户的程序上，而有时候花费在进行系统调用(syscall)上（比如从磁盘或键盘读取数据）。

运行用户程序的时间被标记为「用户时间」，进行系统调用的时间被标记为「系统时间」。二者分别是 `rusage` 的 `ru_utime` 和 `ru_stime` 。

所有涉及访问硬件的行为都是在名为内核模式(Kernel Mode)的特殊模式下完成的。

你的程序不允许直接接触硬件（例如磁盘）；你必须通过请求操作系统来完成接触硬件的操作。

而且，你的程序也不被允许直接进入内核模式。如果你想从磁盘读取文件，那你必须通过请求操作系统提供的特定服务来完成这个行为。

这就是操作系统如何调节硬件的使用。

所以判题程序记录的时间仅记录了 `ru_utime`。

### 常见 `signal` 和 `exit_code`

`signal`(<https://man7.org/linux/man-pages/man7/signal.7.html>):

| signal | code    | 解释                                                                                     |
| ------ | ------- | ---------------------------------------------------------------------------------------- |
| 10     | SIGUSR1 | judge 程序发出的表示程序结束的 signal                                                    |
| 11     | SIGSEGV | 段错误，程序出现空指针，如果是在执行**需要运行时**的程序的时候，可以试试不设置内存限制。 |

`exit_code`:

| exit_code | 解释                               |
| --------- | ---------------------------------- |
| 127       | shell 报出来的 `command not found` |

### 在其他语言中调用

可以以启动子进程的方式来调用 `judge`，然后捕获控制台的输出即可。  
比如：

```python
import json
import subprocess


def judge(proc_args):
    proc = subprocess.Popen(proc_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if err:
        raise ValueError("Error occurred while calling judge: {}".format(err))

    return json.loads(out.decode("utf-8"))


proc_args = [
    "./judge",
    "judge",
    "./main",
    "1000",
    "2048",
    "./tests/1/1.in",
    "./tests/1/1.out",
    "1.tmp.out",
    "-l",
    "1.log",
]

result = judge(proc_args)
print("result: ", result)
# result:  {'status': 0, 'cpu_time_used': 0, 'cpu_time_used_us': 579, 'real_time_used': 1, 'real_time_used_us': 631, 'memory_used': 1500, 'signal': 0, 'exit_code': 0}
```


## 开源致谢

项目中使用到的开源库链接：

- [rxi/log.c](https://github.com/rxi/log.c)

项目开发过程中参考的项目：

- [1510460325/judge-runner](https://github.com/1510460325/judge-runner)
- [dojiong/Lo-runner](https://github.com/dojiong/Lo-runner/)
- [QingdaoU/Judger](https://github.com/QingdaoU/Judger)

对以上项目表示感谢。
