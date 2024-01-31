import { Component } from '@angular/core';
import { RouterLink, RouterLinkActive } from '@angular/router';
import { UsersService } from '../../services/users.service';
import { DevicesService } from '../../services/devices.service';
import { NgFor } from '@angular/common';

@Component({
  selector: 'app-measurements-devices',
  standalone: true,
  imports: [RouterLink, RouterLinkActive, NgFor],
  templateUrl: './measurements-devices.component.html',
  styleUrl: './measurements-devices.component.css'
})
export class MeasurementsDevicesComponent {

  constructor(private usersService: UsersService, private devicesService: DevicesService) {

  }

  getAvailableDevices() {
    return this.devicesService.getAvailableDevices(this.usersService.getCurrentUser());
  }

}
