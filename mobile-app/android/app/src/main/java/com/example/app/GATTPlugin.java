package com.example.app;

import android.Manifest;
import android.content.pm.PackageManager;

import androidx.core.app.ActivityCompat;

import com.espressif.provisioning.ESPConstants;
import com.espressif.provisioning.ESPProvisionManager;
import com.espressif.provisioning.listeners.BleScanListener;
import com.getcapacitor.JSArray;
import com.getcapacitor.JSObject;
import com.getcapacitor.Plugin;
import com.getcapacitor.PluginCall;
import com.getcapacitor.PluginMethod;
import com.getcapacitor.annotation.CapacitorPlugin;

@CapacitorPlugin(name = "GATT")
public class GATTPlugin extends Plugin {

  ESPProvisionManager manager_;
  GattListener listener_ = new GattListener();
  ProvisioningConnector provisioningConnector_;

  BondingConnector bondingConnector_;

  private ESPProvisionManager _getEspManager() {
    if (manager_ == null) {
      manager_ = ESPProvisionManager.getInstance(getContext());
    }

    return manager_;
  }

  @PluginMethod()
  public void startScanning(PluginCall call) {

    if(_getEspManager().getEspDevice() == null) {
      _getEspManager().createESPDevice(ESPConstants.TransportType.TRANSPORT_BLE, ESPConstants.SecurityType.SECURITY_0);
    }

    if (ActivityCompat.checkSelfPermission(getContext(), android.Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
      ActivityCompat.requestPermissions(getActivity(), new String[]{android.Manifest.permission.ACCESS_FINE_LOCATION}, 1234);
      return;
    }

    listener_ = new GattListener();

    _getEspManager().searchBleEspDevices(listener_);

    call.resolve();
  }

  @PluginMethod()
  public void getAvailableDevices(PluginCall call) {

    JSObject ret = new JSObject();
    ret.put("devices", listener_.getDevices());

    call.resolve(ret);
  }

  @PluginMethod()
  public void tryToProvision(PluginCall call) {

    JSObject soughtDevice = call.getObject("device");
    String proofOfPossession = call.getString("proofOfPossession");
    String ssid = call.getString("ssid");
    String password = call.getString("password");

    System.out.println(soughtDevice.toString());
    System.out.println(proofOfPossession);
    System.out.println(ssid);
    System.out.println(password);

    provisioningConnector_ = new ProvisioningConnector(soughtDevice, manager_, ssid, password, proofOfPossession);

    _getEspManager().searchBleEspDevices(provisioningConnector_);

    call.resolve();
  }

  @PluginMethod()
  public void passedCorrectPop(PluginCall call) {

    JSObject ret = new JSObject();
    ret.put("value", provisioningConnector_.passedCorrectPop());

    call.resolve(ret);
  }

  @PluginMethod()
  public void isProvisioningSuccessful(PluginCall call) {

    JSObject ret = new JSObject();
    ret.put("value", provisioningConnector_.provisionedSuccessfully());

    call.resolve(ret);
  }

  @PluginMethod()
  public void tryToBond(PluginCall call) {

    JSObject soughtDevice = call.getObject("device");
    String proofOfPossession = call.getString("proofOfPossession");

    bondingConnector_ = new BondingConnector(soughtDevice, manager_, proofOfPossession);

    _getEspManager().searchBleEspDevices(bondingConnector_);

    call.resolve();
  }

  @PluginMethod()
  public void isBondingSuccessful(PluginCall call) {

    JSObject ret = new JSObject();
    ret.put("value", bondingConnector_.isBondingSuccessful());

    call.resolve(ret);
  }



}
