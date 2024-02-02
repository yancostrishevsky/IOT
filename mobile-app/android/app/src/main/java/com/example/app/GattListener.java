package com.example.app;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanResult;

import com.espressif.provisioning.listeners.BleScanListener;
import com.getcapacitor.JSArray;
import com.getcapacitor.JSObject;

import java.io.Console;

public class GattListener implements BleScanListener {

  final private JSArray devices_ = new JSArray();

  JSArray getDevices() {
    return devices_;
  }

  @Override
  public void scanStartFailed() {}

  @Override
  public void onPeripheralFound(BluetoothDevice device, ScanResult scanResult) {
    System.out.println(device.getName());
    System.out.println(device.getAddress());

    JSObject deviceObj = new JSObject();
    deviceObj.put("name", device.getName());

    devices_.put(deviceObj);
  }

  @Override
  public void scanCompleted() {
    System.out.println("Scan completed!");
  }

  @Override
  public void onFailure(Exception e) {
    System.out.println(e.getMessage());
  }

}
