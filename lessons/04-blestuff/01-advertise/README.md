# 01-advertise (BLE sample: Eddystone beacon)

## About the Sample App

* Simple advertising service with a custom UUID128 service ID
* advertises project name

## To Check

* take the nRF Connect APP and search for a BLE device which advertises as
  "01-advertise"
* check the advertising data with the log (note: reversed byte order)

```
#0[000:00:000.000] 01-advertise
#3[000:00:000.214]       init advsvc
#4[000:00:000.367]         register service/module <SVC|ADV>
#3[000:00:000.550]       init core ...
#2[000:00:000.702]     init tiny BLE wireless core
#3[000:00:000.855]       init bl_ble
#3[000:00:001.008]       init bl_eddy
#4[000:00:001.160]         register service/module <SVC|EDDY>
#1[000:00:001.374]   init app
#4[000:00:001.648]         enable Bluetooth ...
#2[000:00:310.090]     BLE now ready to start
#4[000:00:310.303]         setup Simple Advertising BLE service
#4[000:00:310.486]         BLE receipe: <SVC|ADV>
#4[000:00:310.669]           ap: (advertising parameters)
#4[000:00:310.822]             options: 0x04
#4[000:00:310.975]             interval: 100/150 ms
#4[000:00:361.359]           ad[0]: type: 0x01 #01 | 04
#4[000:00:362.122]           ad[1]: type: 0x07 #16 | 00-88-00-88-00-88-00-88-00-00-88-00-88-00-00-88
#4[000:00:362.336]         total advertising payload: #21 bytes (max 31 bytes allowed)
#4[000:00:363.038]           sd[0]: type: 0x09 #13 | 30-31-2D-61-64-76-65-72-74-69-73-65-00
#4[000:00:363.251]         total scan response payload: #15 bytes (max 31 bytes allowed)
#3[000:00:363.465]       Bluetooth device name: 01-advertise
#3[000:00:364.380]       advertising of BLE service <SVC|ADV> started @ E4:F1:ED:98:90:C5 (random)
#1[000:10:000.977]   I'm alive!
```


## Event Message Flow for Advertising Service Setup

```
   main                 app                   gear
   (M)                  (A)                  (D)(U)
    |                    |                    |  |
+--------------------+   |                    |  |
| bl_service(advsvc) |   |                    |  |  // register advsvc service
+--------------------+   |                    |  |
    |                    |                    |  |
    |         +----------v----------+         |  |  // app defines statically
    |         |  static BL_ble ble; |         |  |  // a data structure <ble>
    |         +----------v----------+         |  |  // to hold all receipe
    |                    |                    |  |
    |                    |      SYS:INIT      |  |  // bl_engine() initializes
    o---------------------------------------->|  |  // wireless core, which re-
    |                    |        (app)       |  |  // gisters advertising svc.
    |                    |                    |  |
    |      SYS:INIT      |                    |  |  // bl_engine() inits app
    o------------------->|     BLE:ENABLE     |  |
    |       (NULL)       o------------------->|  |  // app asks for enabling
    |                    |       <ble>        |  |  // BLE system
    |                    |                    |  |
    |                    |      BLE:READY     |  |  // BLE module notifies
    |                    |<----------------------o  // app that Bluetooth is
    |                    |                    |  |  // now ready
    |         +----------v----------+         |  |
    |         | @sid = BL_SID(ADV_) |         |  |  // define BLE service ID
    |         +----------v----------+         |  |
    |                    |                    |  |
    |                    |   SERVICE:SETUP    |  |  // advise wireless core
    |                    o------------------->|  |  // to setup receipe <ble>
    |                    |     @sid,<ble>     |  |  // for @sid BLE service
    |                    |                    |  |
    |                    |     BLE:START      |  |  // start BLE system by
    |                    o------------------->|  |  // running advertiser
    |                    |       <ble>        |  |  // with <ble> receipe
    |                    |                    |  |
```

## Module Hierarchy and Pipeline Setup

```
   main => bl_engine(app,tick_ms,tock_ms)
    |
    +- app                                     (application)
    |   |<==================================|
   .|.......................................|...................................
    |                                       |
    +- bluccino ===========================>|  (Bluccino module)
        |                                   |
        +- bl_top =========================>|  (top gear)
        |   |                               |
        |   |<========================|     |  // top gear needs some messages
        |                             |     |
        +- bl_up =====================|====>|  (up gear)
        |   |
        |   |<==============================|
        |                                   |
        +- bl_down                          |  (down gear)
            |                               |
   .........|...............................|...................................
            |                               |
            +- bl_core ====================>|  (core system)
                |                           |
                +- bl_wl ==================>|  (tiny BLE wireless core)
                |  |  |<==============|     |
                |  |                  |     |
                |  +- bl_ble ========>|     |  (BLE module)
                |  +- advsvc ========>|     |  (simple advertising BLE service)
                |                           |
                +- bl_hw ==================>|  (empty hardware core)
```

## Test with nRF Connect App

Remember that we provided two kinds of information for the advertising service:

* a service UUID (`88000088-0088-0000-880088008800`), which we provided in the advertising data (`ad[]`)

* a readable name of the service, which matches the project
   name (`01-advertise`) and is provided by the scan response data (`sd[]`)

Using the nRF Connect app on a smart phone shows us these two kinds of information when we scan for the service:
```
    01-advertise
    Services: 88000088-0088-0000-880088008800
```
