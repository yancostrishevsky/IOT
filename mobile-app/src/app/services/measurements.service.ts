import { Injectable } from '@angular/core';
import { AngularFireDatabase } from '@angular/fire/compat/database';
import { HumidityMeasurement, Measurements, TemperatureMeasurement } from '../interfaces/measurements';
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

        const humidityForDevice = measurementsForDevice.humidity;
        const temperatureForDevice = measurementsForDevice.temperature;

        const flattenedHumidity: HumidityMeasurement[] = [];

        Object.keys(humidityForDevice).forEach((humidityKey: any) => {
          flattenedHumidity.push({
            ...humidityForDevice[humidityKey]
          });
        });

        const flattenedTemperature: TemperatureMeasurement[] = [];

        Object.keys(temperatureForDevice).forEach((humidityKey: any) => {
          flattenedTemperature.push({
            ...temperatureForDevice[humidityKey]
          });
        });


        this.measurements[key] = {
          humidity: flattenedHumidity,
          temperature: flattenedTemperature
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
