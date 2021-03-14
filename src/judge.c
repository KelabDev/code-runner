#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "cli.h"
#include "constants.h"
#include "diff.h"
#include "log.h"
#include "run.h"
#include "utils.h"

char result_message[1000];

void init_result(struct Result *result)
{
  result->status = PENDING;
  result->cpu_time_used = result->cpu_time_used_us = 0;
  result->real_time_used = result->real_time_used_us = 0;
  result->memory_used = 0;
  result->signal = result->exit_code = 0;
}

void init_config(struct Config *config)
{
  config->memory_check_only = 0;
  config->cpu_time_limit = config->real_time_limit = config->memory_limit = 0;
  config->log_file = config->in_file = config->out_file = config->stdout_file = '\0';
}

FILE *set_logger(struct Config *config)
{
  FILE *log_fp = NULL;
  log_set_quiet(true);
  if (config->log_file)
  {
    log_fp = fopen(config->log_file, "a");
    if (log_fp == NULL)
    {
      fprintf(stderr, "can not open log file\n");
      fprintf(stderr, "log_file: %s\n", config->log_file);
    }
    else
    {
      log_add_fp(log_fp, 0);
    }
  }
  return log_fp;
}

void log_config(struct Config *config)
{
  int i = 0;
  while ((config->cmd)[i])
  {
    log_debug("config: cmd part %d: %s", i, (config->cmd)[i]);
    i++;
  };
  log_debug("config: cpu_time_limit %d ms", config->cpu_time_limit);
  log_debug("config: real_time_limit %d ms", config->real_time_limit);
  log_debug("config: memory_limit %d kb", config->memory_limit);
  log_debug("config: memory_check_only %d", config->memory_check_only);
  log_debug("config: in_file %s", config->in_file);
  log_debug("config: out_file %s", config->out_file);
  log_debug("config: stdout_file %s", config->stdout_file);
  log_debug("config: log_file %s", config->log_file);
}

void print_result(struct Result *result)
{
  format_result(result_message, result);
  printf("%s\n", result_message);
  log_info(result_message);
}

struct fd_store
{
  int testdata_in_file;
  int testdata_out_file;
  int stdout_file;
  int stderr_file;
};

int main(int argc, char *argv[])
{
  struct Config config;
  struct Result result;

  struct fd_store fs;

  init_config(&config);
  parse_argv(argc, argv, &config);
  FILE *log_fp = set_logger(&config);
  log_config(&config);
  init_result(&result);

  run(&config, &result);
  // 运行失败的话，直接输出结果。
  if (result.exit_code || result.signal)
  {
    print_result(&result);
    CLOSE_FP(log_fp);
    exit(EXIT_FAILURE);
  }
  if (result.status <= ACCEPTED)
    diff(&config, &result);
  print_result(&result);
  CLOSE_FP(log_fp);
  return 0;
}
