package com.example.app;
import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanResult;
import android.os.Handler;

import com.espressif.provisioning.DeviceConnectionEvent;
import com.espressif.provisioning.ESPConstants;
import com.espressif.provisioning.ESPProvisionManager;
import com.espressif.provisioning.listeners.BleScanListener;
import com.espressif.provisioning.listeners.ProvisionListener;
import com.espressif.provisioning.listeners.ResponseListener;
import com.getcapacitor.JSObject;

public class ProvisioningConnector implements BleScanListener {

  JSObject device_;
  ESPProvisionManager manager_;

  String ssid_;
  String password_;

  String proofOfPossession_;

  Handler handler_;

  boolean provisionedSuccessfully_ = false;
  boolean passedCorrectPop_ = false;



  private static final long DEVICE_CONNECT_TIMEOUT = 20000;

  public ProvisioningConnector(JSObject device, ESPProvisionManager manager, String ssid, String password, String proofOfPossession) {
    device_ = device;
    manager_ = manager;
    handler_ = new Handler();
    ssid_ = ssid;
    password_ = password;
    proofOfPossession_ = proofOfPossession;

    EventBus.getDefault().register(this);
  }

  boolean provisionedSuccessfully() {return provisionedSuccessfully_;}

  boolean passedCorrectPop() {return passedCorrectPop_;}

  @Subscribe(threadMode = ThreadMode.MAIN)
  public void onEvent(DeviceConnectionEvent event) {

    System.out.println("On Device Prov Event RECEIVED");

    switch (event.getEventType()) {

      case ESPConstants.EVENT_DEVICE_CONNECTED:
        manager_.getEspDevice().setProofOfPossession(proofOfPossession_);

        manager_.getEspDevice().initSession(new ResponseListener() {
          @Override
          public void onSuccess(byte[] returnData) {

            System.out.println("on session success");

            passedCorrectPop_ = true;

            manager_.getEspDevice().provision(ssid_, password_, new ProvisionListener() {

              @Override
              public void createSessionFailed(Exception e) {
                System.out.println("createSessionFailed");
              }

              @Override
              public void wifiConfigSent() {
                System.out.println("wifiConfigSent");
              }

              @Override
              public void wifiConfigFailed(Exception e) {
                System.out.println("wifiConfigFailed");
              }

              @Override
              public void wifiConfigApplied() {
                System.out.println("wifiConfigApplied");
              }

              @Override
              public void wifiConfigApplyFailed(Exception e) {
                System.out.println("wifiConfigApplyFailed");
              }

              @Override
              public void provisioningFailedFromDevice(ESPConstants.ProvisionFailureReason failureReason) {
                System.out.println("provisioningFailedFromDevice");
              }

              @Override
              public void deviceProvisioningSuccess() {
                provisionedSuccessfully_ = true;
              }

              @Override
              public void onProvisioningFailed(Exception e) {
                System.out.println("PROVISIONING FAILED");
              }
            });
            }

          @Override
          public void onFailure(Exception e) {
            System.out.println("Failed to create session");
          }

        });



        break;

      case ESPConstants.EVENT_DEVICE_CONNECTION_FAILED:
        break;
    }

    EventBus.getDefault().unregister(this);
  }

  @Override
  public void scanStartFailed() {}

  @Override
  public void onPeripheralFound(BluetoothDevice device, ScanResult scanResult) {

    if(device_.getString("address").equals(device.getAddress())){

      System.out.println(device.getName());
      System.out.println(device.getAddress());
      System.out.println(String.format("Service uuid: %s", scanResult.getScanRecord().getServiceUuids().get(0).toString()));

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
