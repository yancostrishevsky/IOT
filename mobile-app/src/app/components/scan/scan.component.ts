import { Component } from '@angular/core';
import { RouterLink, RouterLinkActive } from '@angular/router';
import GATT, { GATTPlugin, GattDevice } from '../../plugins/GATTPlugin';
import { NgFor, NgIf } from '@angular/common';

@Component({
  selector: 'app-scan',
  standalone: true,
  imports: [RouterLink, RouterLinkActive, NgFor, NgIf],
  templateUrl: './scan.component.html',
  styleUrl: './scan.component.css'
})
export class ScanComponent {

  devices: GattDevice[] = [];
  isScanningNow: boolean = false;

  constructor() {

  }

  collectDevices() {

    this.isScanningNow = false;

    GATT.getAvailableDevices().then((res) => {
      this.devices = res.devices;
    });
  }

  scan() {

    this.devices = [];

    GATT.startScanning().then((res) => {
      this.isScanningNow = true;
    });

    setTimeout(() => { this.collectDevices() }, 6000);
  }

}
