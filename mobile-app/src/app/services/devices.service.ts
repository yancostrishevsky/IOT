import { Injectable } from '@angular/core';
import { AngularFireDatabase } from '@angular/fire/compat/database';
import { Device } from '../interfaces/device';
import { User } from '../interfaces/user';

@Injectable({
  providedIn: 'root'
})
export class DevicesService {

  devices: Device[] = [];

  constructor(private db: AngularFireDatabase) {

    this.db.object('devices').valueChanges().subscribe((data: any) => {

      if (data == null) {
        data = {};
      }

      this.devices = [];

      Object.keys(data).forEach((key: any) => {
        this.devices.push(
          {
            ...data[key]
          }
        );
      });


    });
  }

  getAvailableDevices(user: User | null) {

    if (user == null) {
      return [];
    }

    return user.availableDevices.map((deviceName: string) => {
      return this.devices.find((device: Device) => {
        return device.name == deviceName;
      });
    });
  }

  getDevice(name: string): Device | null {
    return this.devices.find((device: Device) => {
      return device.name == name;
    }) || null;
  }

  getDeviceByAddress(address: string): Device | null {
    return this.devices.find((device: Device) => {
      return device.mac == address;
    }) || null;
  }
}
