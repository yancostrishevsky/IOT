import { NgFor, NgIf } from '@angular/common';
import { Component } from '@angular/core';
import { RouterLink, RouterLinkActive } from '@angular/router';
import { UsersService } from '../../services/users.service';
import { DevicesService } from '../../services/devices.service';
import { AngularFireDatabase } from '@angular/fire/compat/database';
import { Device } from '../../interfaces/device';

@Component({
  selector: 'app-configure-devices',
  standalone: true,
  imports: [RouterLink, RouterLinkActive, NgFor, NgIf],
  templateUrl: './configure-devices.component.html',
  styleUrl: './configure-devices.component.css'
})
export class ConfigureDevicesComponent {

  constructor(private usersService: UsersService, private devicesService: DevicesService, private db: AngularFireDatabase) {

  }

  getDevices() {
    if (this.usersService.getCurrentUser() != null) {
      return this.usersService.getCurrentUser()?.availableDevices.map((deviceName: string) => this.devicesService.getDevice(deviceName));
    }

    return [];
  }

  getDeviceOwners(device: Device | null) {
    return this.usersService.getUsers().filter((user) => user.availableDevices.includes(device?.name || ''));
  }

  removeDeviceOwner(device: Device | null, user: any) {
    this.usersService.deleteOwnership(user, device?.name || '');
  }

}
