# 02-ebeacon (BLE sample: Eddystone beacon)

## About the Sample App

* Eddystone beacon sample with advertising data configuration on a separate
  source file

## Event Message Flow for Advertising Service Setup

```
   main                 app                   gear
   (M)                  (A)                  (D)(U)
    |                    |                    |  |
    |         +----------v----------+         |  |  // app defines statically
    |         |  static BL_ble ble; |         |  |  // a data structure <ble>
    |         +----------v----------+         |  |  // to hold all receipe
    |                    |                    |  |
    |                    |      SYS:INIT      |  |  // bl_engine() initializes
    o---------------------------------------->|  |  // wireless core, which re-
    |                    |        (app)       |  |  // gisters Eddystone service
    |                    |                    |  |
    |      SYS:INIT      |                    |  |  // bl_engine() inits app
    o------------------->|     BLE:ENABLE     |  |
    |       (NULL)       o------------------->|  |  // app asks for enabling
    |                    |       <ble>        |  |  // BLE system
    |                    |                    |  |
    |                    |      BLE:READY     |  |  // BLE module notifies
    |                    |<----------------------o  // app that Bluetooth is
    |                    |                    |  |  // now ready
    |                    |                    |  |
    |                    |  SERVICE:EDDYSTONE |  |  // advise wireless core
    |                    o------------------->|  |  // to setup receipe <ble>
    |                    |       <ble>        |  |  // for Eddystone service
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
                |  +- bl_eddy =======>|     |  (Eddystone BLE service)
                |  +- bl_ibeacon ====>|     |  (iBeacon BLE service)
                |  +- bl_bas ========>|     |  (battery BLE service)
                |                           |
                +- bl_hw ==================>|  (empty hardware core)
```

## Test with nRF Connect App

nRF Connect scanner should display an entry as follows:
```
     01-ebeacon
     Data: http://www.bluccino.org
     Services: Eddystone
     Tx Power: -40 dBm
     Frame Type: 0x10 (URL)
     Service Data: Eddystone
     0x08-2E-6F-6E-69-63-63-75-6C-62-00-D8-10
```
