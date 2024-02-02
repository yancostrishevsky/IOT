package com.example.app;

import android.Manifest;
import android.content.pm.PackageManager;

import androidx.core.app.ActivityCompat;

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

  private ESPProvisionManager _getEspManager() {
    if (manager_ == null) {
      manager_ = ESPProvisionManager.getInstance(getContext());
    }

    return manager_;
  }

  @PluginMethod()
  public void startScanning(PluginCall call) {
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
}
