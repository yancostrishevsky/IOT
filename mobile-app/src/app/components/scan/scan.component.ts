import { Component } from '@angular/core';
import { RouterLink, RouterLinkActive } from '@angular/router';
import GATT, { GATTPlugin, GattDevice } from '../../plugins/GATTPlugin';
import { NgFor, NgIf } from '@angular/common';
import { UsersService } from '../../services/users.service';
import { FormControl, FormGroup, ReactiveFormsModule, Validators } from '@angular/forms';
import { DevicesService } from '../../services/devices.service';

@Component({
  selector: 'app-scan',
  standalone: true,
  imports: [RouterLink, RouterLinkActive, NgFor, NgIf, ReactiveFormsModule],
  templateUrl: './scan.component.html',
  styleUrl: './scan.component.css'
})
export class ScanComponent {

  devices: GattDevice[] = [];
  isScanningNow: boolean = false;
  isConnectingNow: boolean = false;
  connectingTo: GattDevice | null = null;

  provisioningForm: FormGroup;
  bondingForm: FormGroup;

  provisioningDialogDevice: GattDevice | null = null;
  bondingDialogDevice: GattDevice | null = null;

  successMessage: any = {};
  errorMessage: any = {};


  constructor(private usersService: UsersService, private devicesService: DevicesService) {

    this.provisioningForm = new FormGroup({
      ssid: new FormControl('', Validators.required),
      password: new FormControl('', Validators.required),
      pop: new FormControl('', Validators.required)
    });


    this.bondingForm = new FormGroup({
      pop: new FormControl('', Validators.required)
    });

  }

  collectDevices() {

    this.isScanningNow = false;

    GATT.getAvailableDevices().then((res) => {

      this.devices = res.devices;

      this.devices.map((device) => {
        if (this.devicesService.getDeviceByAddress(device.address) == null) {
          device.isConnectable = false;
        }
      });

    });
  }

  isAlreadyPaired(device: GattDevice) {
    return this.usersService.currentUser?.availableDevices.find((d) => this.devicesService.getDevice(d) == this.devicesService.getDeviceByAddress(device.address)) != null;
  }

  scan() {

    this.errorMessage = {};
    this.successMessage = {};

    this.devices = [];

    GATT.startScanning().then((res) => {
      this.isScanningNow = true;
      setTimeout(() => { this.collectDevices() }, 6000);
    });


  }

  toggleProvisioningDialog(device: GattDevice) {

    this.successMessage = {};
    this.errorMessage = {};

    this.bondingDialogDevice = null;

    if (this.provisioningDialogDevice === device) {
      this.provisioningDialogDevice = null;
      return;
    }

    this.provisioningDialogDevice = device;
  }

  toggleBondingDialog(device: GattDevice) {

    this.successMessage = {};
    this.errorMessage = {};

    this.provisioningDialogDevice = null;

    if (this.bondingDialogDevice === device) {
      this.bondingDialogDevice = null;
      return;
    }

    this.bondingDialogDevice = device;
  }

  checkProvisioningStatus(device: GattDevice) {

    this.isConnectingNow = false;
    this.connectingTo = null;

    GATT.isProvisioningSuccessful().then((res) => {
      if (res.value) {
        this.successMessage = { device: device, message: "Provisioning successful." };
      }
      else {
        GATT.passedCorrectPop().then((res) => {
          if (res.value) {
            this.errorMessage = { device: device, message: "Provisioning failed. Probably wrong credentials." };
          }
          else {
            this.errorMessage = { device: device, message: "Provisioning failed. Probably wrong POP." };
          }
        });
      }
    });
  }


  tryProvision(device: GattDevice) {
    this.successMessage = {};
    this.errorMessage = {};

    this.isConnectingNow = true;
    this.provisioningDialogDevice = null;
    this.connectingTo = device;



    GATT.tryToProvision({ device: device, proofOfPossession: this.provisioningForm.value.pop, ssid: this.provisioningForm.value.ssid, password: this.provisioningForm.value.password }).then((res) => {
      setTimeout(() => { this.checkProvisioningStatus(device) }, 20000);
    });

    this.provisioningForm.reset();

  }

  checkBondingStatus(device: GattDevice) {

    this.isConnectingNow = false;
    this.connectingTo = null;

    GATT.isBondingSuccessful().then((res) => {
      if (res.value) {
        this.successMessage = { device: device, message: "Bonding successful." };

        const deviceName = this.devicesService.getDeviceByAddress(device.address)?.name || "";

        if (deviceName != "") {

          this.usersService.addOwnership(this.usersService.currentUser, deviceName);
        }
      }
      else {
        this.errorMessage = { device: device, message: "Bonding failed." };
      }
    });
  }

  tryBond(device: GattDevice) {

    this.successMessage = {};
    this.errorMessage = {};

    this.isConnectingNow = true;
    this.bondingDialogDevice = null;
    this.connectingTo = device;


    GATT.tryToBond({ device: device, proofOfPossession: this.bondingForm.value.pop }).then((res) => {
      setTimeout(() => { this.checkBondingStatus(device) }, 7000);
    });

    this.bondingForm.reset();

  }

}
