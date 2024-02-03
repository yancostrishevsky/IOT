package com.example.app;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanResult;
import android.os.ParcelUuid;

import com.espressif.provisioning.ESPProvisionManager;
import com.espressif.provisioning.listeners.BleScanListener;
import com.getcapacitor.JSArray;
import com.getcapacitor.JSObject;

import java.io.Console;

public class GattConnector implements BleScanListener {

  JSObject device_;
  ESPProvisionManager manager_;

  GattConnector(JSObject device, ESPProvisionManager manager) {
    device_ = device;
    manager_ = manager;
  }

  @Override
  public void scanStartFailed() {}

  @Override
  public void onPeripheralFound(BluetoothDevice device, ScanResult scanResult) {
    System.out.println(device.getName());
    System.out.println(device.getAddress());
    System.out.println(device_.getString("address"));
//

    if(device_.getString("address").equals(device.getAddress())){

      String primaryUuid = scanResult.getScanRecord().getServiceUuids().get(0).toString();
      manager_.getEspDevice().connectBLEDevice(device, primaryUuid);

    }

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
