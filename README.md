# LoNHttpfw
基于LoNetfw网络框架的的Http框架

### AB压力测试结果
``` shell
ab -n 1000000 -c 200 "http://127.0.0.1:8080/"
```
1. 编译版本：Debug 优化等级：O0
```
Server Software:        http_server/LoNetfw-1.0.0
Server Hostname:        127.0.0.1
Server Port:            8080

Document Path:          /
Document Length:        804 bytes

Concurrency Level:      200
Time taken for tests:   104.490 seconds
Complete requests:      1000000
Failed requests:        0
Total transferred:      923000000 bytes
HTML transferred:       804000000 bytes
Requests per second:    9570.34 [#/sec] (mean)
Time per request:       20.898 [ms] (mean)
Time per request:       0.104 [ms] (mean, across all concurrent requests)
Transfer rate:          8626.39 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.8      0      18
Processing:     1   21   3.3     20      57
Waiting:        1   21   3.2     20      57
Total:          3   21   3.1     20      57

Percentage of the requests served within a certain time (ms)
  50%     20
  66%     20
  75%     21
  80%     23
  90%     24
  95%     27
  98%     29
  99%     33
 100%     57 (longest request)
```

2. 编译版本：Release 优化等级：O3 (推荐)
```
Server Software:        http_server/LoNetfw-1.0.0
Server Hostname:        127.0.0.1
Server Port:            8080

Document Path:          /
Document Length:        804 bytes

Concurrency Level:      200
Time taken for tests:   40.479 seconds
Complete requests:      1000000
Failed requests:        0
Total transferred:      923000000 bytes
HTML transferred:       804000000 bytes
Requests per second:    24703.90 [#/sec] (mean)
Time per request:       8.096 [ms] (mean)
Time per request:       0.040 [ms] (mean, across all concurrent requests)
Transfer rate:          22267.28 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    3   0.8      3      16
Processing:     1    5   1.3      5      28
Waiting:        0    4   1.2      4      22
Total:          2    8   1.4      8      29

Percentage of the requests served within a certain time (ms)
  50%      8
  66%      8
  75%      8
  80%      8
  90%      9
  95%     11
  98%     12
  99%     13
 100%     29 (longest request)
```