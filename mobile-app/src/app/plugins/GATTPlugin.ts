import { registerPlugin } from "@capacitor/core";

export interface GattDevice {
  name: string;
  address: string;
};

export interface GATTPlugin {
  getAvailableDevices(): Promise<{ devices: GattDevice[] }>;
  startScanning(): Promise<{}>;
  connectToDevice(device: GattDevice): Promise<{}>;
}

class MockGATT implements GATTPlugin {

  async getAvailableDevices(): Promise<{ devices: GattDevice[] }> {
    return {
      devices: [
        { name: "Device 1", address: "00:00:00:00:00:00" },
        { name: "Device 2", address: "00:00:00:00:00:01" },
        { name: "Device 3", address: "00:00:00:00:00:02" },
        { name: "Device 4", address: "00:00:00:00:00:03" },
        { name: "Device 5", address: "00:00:00:00:00:04" },
        { name: "Device 6", address: "00:00:00:00:00:05" },
        { name: "Device 7", address: "00:00:00:00:00:06" },
        { name: "Device 8", address: "00:00:00:00:00:07" },
        { name: "Device 9", address: "00:00:00:00:00:08" },
      ]
    };
  }

  async startScanning(): Promise<{}> {
    return {};
  }

  async connectToDevice(device: GattDevice): Promise<{}> {
    return {};
  }
}


const GATT = registerPlugin<GATTPlugin>("GATT");
// const GATT = new MockGATT();

export default GATT;
