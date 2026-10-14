#ifndef PERF_H
#define PERF_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include "kupyna.h"
#include <stdio.h>
#include <stddef.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>

typedef struct {
  struct timespec wall_start;
  struct timespec wall_end;
  struct timespec cpu_start;
  struct timespec cpu_end;
  struct rusage rusage_start;
  struct rusage rusage_end;
  uint64_t cycles_start;
  uint64_t cycles_end;
  size_t bytes;
} perf_meter_t;

static inline uint64_t perf_rdtsc(void) {
  unsigned int lo, hi;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

static inline void perf_start(perf_meter_t *m) {
  m->bytes = 0;
  getrusage(RUSAGE_SELF, &m->rusage_start);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m->cpu_start);
  clock_gettime(CLOCK_MONOTONIC, &m->wall_start);
  m->cycles_start = perf_rdtsc();
}

static inline void perf_stop(perf_meter_t *m, size_t bytes) {
  m->cycles_end = perf_rdtsc();
  clock_gettime(CLOCK_MONOTONIC, &m->wall_end);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m->cpu_end);
  getrusage(RUSAGE_SELF, &m->rusage_end);
  m->bytes = bytes;
}

static inline double perf_get_wall_time_ms(const perf_meter_t *m) {
  double sec = (double)(m->wall_end.tv_sec - m->wall_start.tv_sec);
  double nsec = (double)(m->wall_end.tv_nsec - m->wall_start.tv_nsec);
  return sec * 1000.0 + nsec / 1e6;
}

static inline double perf_get_cpu_time_ms(const perf_meter_t *m) {
  double sec = (double)(m->cpu_end.tv_sec - m->cpu_start.tv_sec);
  double nsec = (double)(m->cpu_end.tv_nsec - m->cpu_start.tv_nsec);
  return sec * 1000.0 + nsec / 1e6;
}

static inline double perf_get_user_time_ms(const perf_meter_t *m) {
  double sec =
      (double)(m->rusage_end.ru_utime.tv_sec - m->rusage_start.ru_utime.tv_sec);
  double usec = (double)(m->rusage_end.ru_utime.tv_usec -
                         m->rusage_start.ru_utime.tv_usec);
  return sec * 1000.0 + usec / 1000.0;
}

static inline double perf_get_sys_time_ms(const perf_meter_t *m) {
  double sec =
      (double)(m->rusage_end.ru_stime.tv_sec - m->rusage_start.ru_stime.tv_sec);
  double usec = (double)(m->rusage_end.ru_stime.tv_usec -
                         m->rusage_start.ru_stime.tv_usec);
  return sec * 1000.0 + usec / 1000.0;
}

static inline uint64_t perf_get_cycles(const perf_meter_t *m) {
  return (m->cycles_end >= m->cycles_start) ? (m->cycles_end - m->cycles_start)
                                            : 0;
}

static inline void perf_print(const perf_meter_t *m, const char *label) {
  double wall_ms = perf_get_wall_time_ms(m);
  double cpu_ms = perf_get_cpu_time_ms(m);
  double user_ms = perf_get_user_time_ms(m);
  double sys_ms = perf_get_sys_time_ms(m);
  uint64_t cycles = perf_get_cycles(m);
  double cpu_load_pct =
      (wall_ms > 0.0001) ? ((cpu_ms / wall_ms) * 100.0) : 100.0;
  if (cpu_load_pct > 100.0)
    cpu_load_pct = 100.0;

  printf("\n  [Метрики навантаження: %s]\n", label);
  printf("  + Оброблено даних:           %zu байт (%.2f КБ)\n", m->bytes,
         (double)m->bytes / 1024.0);
  printf("  + Процесорний час (CPU Time):%.4f мс (User: %.4f мс, System: %.4f "
         "мс)\n",
         cpu_ms, user_ms, sys_ms);
  printf("  + Реальний час (Wall Clock): %.4f мс\n", wall_ms);
  if (wall_ms > cpu_ms * 5.0 && cpu_ms > 0.0) {
    printf("  + Завантаження CPU (Load):   %.2f %% (занижено через паузу в налагоджувачі)\n", cpu_load_pct);
  } else {
    printf("  + Завантаження CPU (Load):   %.1f %%\n", cpu_load_pct);
  }
  printf("  + Такти процесора (Cycles):  %llu тактів\n",
         (unsigned long long)cycles);
  if (m->bytes > 0) {
    double cpb = (double)cycles / (double)m->bytes;
    double effective_time_sec = (cpu_ms > 0.0001) ? (cpu_ms / 1000.0) : (wall_ms / 1000.0);
    double speed_kbs = (effective_time_sec > 0.0) ? (((double)m->bytes / 1024.0) / effective_time_sec) : 0.0;
    double speed_mbs = speed_kbs / 1024.0;
    double speed_mbits = speed_mbs * 8.0;

    printf("  + Такти на байт (CPB):       %.2f cycles/byte\n", cpb);
    if (speed_mbs >= 0.01) {
      printf("  + Пропускна здатність (CPU): %.2f МБ/с (%.2f Мбіт/с)\n", speed_mbs, speed_mbits);
    } else {
      printf("  + Пропускна здатність (CPU): %.2f КБ/с (%.4f МБ/с)\n", speed_kbs, speed_mbs);
    }
  }
  printf("  + Пікова пам'ять (Max RSS):  %ld КБ\n", m->rusage_end.ru_maxrss);
}

#endif
