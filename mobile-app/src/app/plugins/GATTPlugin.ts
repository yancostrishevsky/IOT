import { registerPlugin } from "@capacitor/core";

export interface GattDevice {
  name: string;
};

export interface GATTPlugin {
  getAvailableDevices(): Promise<{devices: GattDevice[]}>;
  startScanning() : Promise<{}>;
}

const GATT = registerPlugin<GATTPlugin>("GATT");

export default GATT;
