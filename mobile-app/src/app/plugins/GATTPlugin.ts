import { registerPlugin } from "@capacitor/core";

export interface GattDevice {
  name: string;
  address: string;
  isConnectable: boolean;
};

export interface GATTPlugin {
  getAvailableDevices(): Promise<{ devices: GattDevice[] }>;
  startScanning(): Promise<{}>;

  tryToProvision(options: { device: GattDevice, proofOfPossession: String, ssid: String, password: String }): Promise<{}>;
  passedCorrectPop(): Promise<{ value: boolean }>;
  isProvisioningSuccessful(): Promise<{ value: boolean }>;

  tryToBond(options: { device: GattDevice, proofOfPossession: boolean }): Promise<{}>;
  isBondingSuccessful(): Promise<{ value: boolean }>;
}

class MockGATT implements GATTPlugin {

  async getAvailableDevices(): Promise<{ devices: GattDevice[] }> {
    return {
      devices: [
        { name: "device1", address: "00:00:00:00:00:00", isConnectable: true },
        { name: "device43", address: "00:00:00:00:00:01", isConnectable: false },
        { name: "device2", address: "00:00:00:00:00:02", isConnectable: true },
      ]
    };
  }

  async startScanning(): Promise<{}> {
    return {};
  }

  async connectToDevice(device: GattDevice): Promise<{}> {
    return {};
  }

  async tryToProvision(options: { device: GattDevice, proofOfPossession: String, ssid: String, password: String }): Promise<{}> { return {}; }

  async passedCorrectPop(): Promise<{ value: boolean }> {
    return { value: false };
  }

  async isProvisioningSuccessful(): Promise<{ value: boolean }> {
    return { value: false };
  }

  async tryToBond(options: { device: GattDevice, proofOfPossession: boolean }): Promise<{}> { return {}; }

  async isBondingSuccessful(): Promise<{ value: boolean }> {
    return { value: true };
  }


}


const GATT = registerPlugin<GATTPlugin>("GATT");
// const GATT = new MockGATT();

export default GATT;
