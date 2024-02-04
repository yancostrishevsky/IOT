import { Injectable } from '@angular/core';
import { AngularFireDatabase } from '@angular/fire/compat/database';
import { HumidityMeasurement, Measurements, MovementMeasurement, TemperatureMeasurement } from '../interfaces/measurements';
import { Device } from '../interfaces/device';

@Injectable({
  providedIn: 'root'
})
export class MeasurementsService {

  measurements: { [key: string]: Measurements } = {};

  constructor(private db: AngularFireDatabase) {

    this.db.object('measurements').valueChanges().subscribe((data: any) => {

      if (data == null) {
        data = {};
      }

      this.measurements = {};

      Object.keys(data).forEach((key: any) => {

        const measurementsForDevice = data[key];

        const humidityForDevice = measurementsForDevice.humidity || {};
        const temperatureForDevice = measurementsForDevice.temperature || {};
        const movementForDevice = measurementsForDevice.movement || {};

        const flattenedHumidity: HumidityMeasurement[] = [];

        Object.keys(humidityForDevice).forEach((humidityKey: any) => {
          flattenedHumidity.push({
            ...humidityForDevice[humidityKey]
          });
        });

        const flattenedTemperature: TemperatureMeasurement[] = [];

        Object.keys(temperatureForDevice).forEach((temperatureKey: any) => {
          flattenedTemperature.push({
            ...temperatureForDevice[temperatureKey]
          });
        });

        const flattenedMovement: MovementMeasurement[] = [];

        Object.keys(movementForDevice).forEach((movementKey: any) => {
          flattenedMovement.push({
            ...movementForDevice[movementKey]
          });
        });

        this.measurements[key] = {
          humidity: flattenedHumidity,
          temperature: flattenedTemperature,
          movement: flattenedMovement
        };
      });


    });
  }

  getMeasurements(device: Device | null): Measurements | null {

    if (device == null) {
      return null;
    }

    return this.measurements[device.name] || null;
  }
}
