# Power Consumption Benchmarking

The power consumption of ENTS was found by supplying nominal voltages of batteries in place of the developed battery board. A Kiethley 2450 SourceMeter was used to measure the current draw of the system at various voltages. The following graphs show the traces and current draw for (1) uploading via LoRaWAN, (2) measurements, and (3) sleep states. The esp32 was forced into a deep sleep state as its usage is not recommended during field deployments and would significantly increase the power consumption of the system.

## Battery Voltage 3.9V

```
Metrics for overall range (0.0 mA to 200.0 mA):
count    100000.000000
mean          5.958600
std           9.764411
min           2.599716
25%           4.688263
50%           4.688263
75%           4.758835
max         145.168304
Name: Reading, dtype: float64

Metrics for idle range (0.0 mA to 10.0 mA):
count    96372.000000
mean         4.819782
std          0.489174
min          2.599716
25%          4.688263
50%          4.688263
75%          4.758835
max          9.981155
Name: Reading, dtype: float64

Metrics for measure range (10.0 mA to 20.0 mA):
count    2635.000000
mean       11.811225
std         2.501169
min        10.051727
25%        10.887146
50%        11.026382
75%        11.165619
max        19.941330
Name: Reading, dtype: float64

Metrics for comm range (80.0 mA to 200.0 mA):
count    946.000000
mean     104.873724
std        3.319625
min       95.718384
25%      103.588104
50%      103.797913
75%      104.284286
max      145.168304
Name: Reading, dtype: float64
```

![Power Consumption 3.9V](./3.9.jpg)

## Battery Voltage 3.7V

```
Metrics for overall range (0.0 mA to 200.0 mA):
count    100000.000000
mean          5.655935
std           8.150841
min           2.529144
25%           4.619598
50%           4.619598
75%           4.688263
max         130.403519
Name: Reading, dtype: float64

Metrics for idle range (0.0 mA to 10.0 mA):
count    96142.000000
mean         4.644379
std          0.176496
min          2.529144
25%          4.619598
50%          4.619598
75%          4.688263
max          9.981155
Name: Reading, dtype: float64

Metrics for measure range (10.0 mA to 20.0 mA):
count    2801.000000
mean       11.678816
std         2.394680
min        10.051727
25%        10.818481
50%        10.957718
75%        11.096954
max        19.941330
Name: Reading, dtype: float64

Metrics for comm range (80.0 mA to 200.0 mA):
count    974.000000
mean      86.524535
std        2.890134
min       81.510544
25%       85.479736
50%       85.758209
75%       86.576462
max      130.403519
Name: Reading, dtype: float64
```

![Power Consumption 3.7V](./3.7.jpg)
