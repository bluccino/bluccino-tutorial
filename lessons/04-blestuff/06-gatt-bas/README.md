# 06-gatt-bas - BLE peripheral which offers connection to battery service (BAS)

## About the Sample App

* 04-bas is derived from 04-peripheral (originally based on the Zephyr
  bluetooth/peripheral sample

## Event Message Flow for Advertising Service Setup

After the `app` gets a system initializing request `[SYS:INIT (cb)]` from `main`
the first task of the `app` is to enable Bluetooth by sending a `[BLE:ENABLE
<BL_ble>]` message to `bl_ble`, which will first initialize the provided
`<BL_ble>` data structure and (usually) respond with a `[BLE:READY]` message,
to be dispatched by the app in order to run the Bluetooth service setup
procedure and to start the advertiser.

After receiving `[BLE:READY]` the app sends a `[SERVICE:VENDOR <BL_ble>]`
message to the `vendor` module (vendor BLE service) which will setup the
advertising data of the BLE receipe with HRS/BAS/CIS and a vendor service.

The app provides the data container `<BL_ble>` into which the vendor service
will store the pointers to the static data `<BL_ble>` defining the vendor
service. An error return value tells the `app`, whether the service setup was
successful or not.

After successful setup of the vendor service the `app` will start the
advertiser based on the receipe stored in `<BL_ble>`.

```
// 01-peripheral.b

  M := main;        // set verbose level, print hello msg., run tick/tock engine
  A := app;         // application top module
--------------------------------------------------------------------------------
  B := bl_ble;      // interface to Zephyr Bluetooth/GAP/GATT interface
    C := cts;       // current time service
    V := vendor;    // vendor specific service setup, including HRS/BAT/CTS

// initializing message flow

  M -> [SYS:INIT (app)] -> B;             // during core/driver init
  M -> [SYS:INIT (NULL)] -> A;
  A -> [BLE:ENABLE]-> B;                  // bt_enable(NULL)
        B:   { bt_ready(); }
        B:   { settings_load(); }
        B -> [BLE:READY] -> A;

    // Bluetooth is initialized and ready!
    // setup the advertising parameters and advertising data ...

  A -> [SERVICE:VENDOR] -> V;
        V -> [CTS:INIT] -> C;
              C:   { cts_init(); }

    // register GATT and SM callbacks

        V:  { bt_gatt_cb_register(); }
        V:  { bt_conn_auth_cb_register(); }
        V:  { t_gatt_find_by_uuid(); }

    // all prepared: now 1) start advertising and initiate gatt register process

  A -> [BLE:START <BL_ble>] -> B;
        B: { bt_le_adv_start(); }

  M -> [SYS_TOCK @ix,<BL_pace>,cnt] -> A;
        A: { cts_notify(); }
        A: { hrs_notify(); }
        A: { bas_notify(); }
```
