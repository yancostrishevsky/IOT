import { Component } from '@angular/core';
import { ActivatedRoute, RouterLink, RouterLinkActive } from '@angular/router';
import { Device } from '../../interfaces/device';
import { Measurements } from '../../interfaces/measurements';
import { MeasurementsService } from '../../services/measurements.service';
import { DevicesService } from '../../services/devices.service';
import { AngularFireDatabase } from '@angular/fire/compat/database';
import { NgFor } from '@angular/common';

@Component({
  selector: 'app-measurements',
  standalone: true,
  imports: [RouterLink, RouterLinkActive, NgFor],
  templateUrl: './measurements.component.html',
  styleUrl: './measurements.component.css'
})
export class MeasurementsComponent {

  device: Device | null = null;
  measurements: Measurements | null = null;

  constructor(private activatedRoute: ActivatedRoute, private measurementsService: MeasurementsService, private devicesService: DevicesService, private db: AngularFireDatabase) {

    this.activatedRoute.paramMap.subscribe(params => {
      let deviceName = params.get('name') || "";

      this.db.object('devices').valueChanges().subscribe((data: any) => {
        this.device = this.devicesService.getDevice(deviceName);

        this.db.object('measurements').valueChanges().subscribe((data: any) => {

          this.measurements = this.measurementsService.getMeasurements(this.device);

        });

      });


    });
  }
}
