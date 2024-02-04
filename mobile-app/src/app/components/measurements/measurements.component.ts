import { Component } from '@angular/core';
import { ActivatedRoute, RouterLink, RouterLinkActive } from '@angular/router';
import { Device } from '../../interfaces/device';
import { HumidityMeasurement, Measurements, MovementMeasurement } from '../../interfaces/measurements';
import { MeasurementsService } from '../../services/measurements.service';
import { DevicesService } from '../../services/devices.service';
import { AngularFireDatabase } from '@angular/fire/compat/database';
import { NgFor, NgIf } from '@angular/common';
import { Chart } from 'chart.js/auto';

@Component({
  selector: 'app-measurements',
  standalone: true,
  imports: [RouterLink, RouterLinkActive, NgFor, NgIf],
  templateUrl: './measurements.component.html',
  styleUrl: './measurements.component.css'
})
export class MeasurementsComponent {

  device: Device | null = null;
  measurements: Measurements | null = null;
  humidityChart: any = null;
  temperatureChart: any = null;
  movementChart: any = null;

  constructor(private activatedRoute: ActivatedRoute, private measurementsService: MeasurementsService, private devicesService: DevicesService, private db: AngularFireDatabase) {

    this.activatedRoute.paramMap.subscribe(params => {
      let deviceName = params.get('name') || "";

      this.db.object('devices').valueChanges().subscribe((data: any) => {
        this.device = this.devicesService.getDevice(deviceName);

        this.db.object('measurements').valueChanges().subscribe((data: any) => {

          this.measurements = this.measurementsService.getMeasurements(this.device);

          this.updateHumidityChart();
          this.updateTemperatureChart();
          this.updateMovementChart();
        });
      });
    });
  }

  ngOnChanges() {
    if (this.measurements) {
      this.updateHumidityChart();
      this.updateTemperatureChart();
      this.updateMovementChart();
    }
  }

  timestampToDate(timestamp: number) {
    const date = new Date(Math.round(timestamp * 1000));

    return date.toISOString().split('T')[0] + ' ' + date.toISOString().split('T')[1].split('.')[0];
  }

  updateHumidityChart() {
    if (this.measurements) {

      const humidityData = this.measurements.humidity;
      const labels = humidityData.map((measurement: HumidityMeasurement) => this.timestampToDate(measurement.timestamp));
      const values = humidityData.map((measurement: HumidityMeasurement) => measurement.value);

      this.humidityChart = new Chart('humidityChart', {
        type: 'bar',
        data: {
          labels: labels,
          datasets: [
            {
              label: 'Humidity (%)',
              data: values,
              backgroundColor: 'rgba(75, 192, 192, 0.2)',
              borderColor: 'rgba(75, 192, 192, 1)',
              borderWidth: 1
            }
          ]
        },
        options: {
          scales: {
            y: {
              beginAtZero: true,
              max: 100,
            }
          },
          responsive: true,
          maintainAspectRatio: false
        }
      });
    }
  }

  updateTemperatureChart() {

    if (this.measurements) {
      const temperatureData = this.measurements.temperature;
      const labels = temperatureData.map((measurement: HumidityMeasurement) => this.timestampToDate(measurement.timestamp));
      const values = temperatureData.map((measurement: HumidityMeasurement) => measurement.value);

      this.temperatureChart = new Chart('temperatureChart', {
        type: 'line',
        data: {
          labels: labels,
          datasets: [
            {
              label: 'Temperature (Celsius degrees)',
              data: values,
              backgroundColor: 'rgba(255, 99, 132, 0.2)',
              borderColor: 'rgba(255, 99, 132, 1)',
              borderWidth: 1
            }
          ]
        },
        options: {
          scales: {
            y: {
              beginAtZero: true,
            }
          },
          responsive: true,
          maintainAspectRatio: false
        }
      });
    }
  }

  updateMovementChart() {
    if (this.measurements) {
      const movementData = this.measurements.movement;
      const labels = movementData.map((measurement: MovementMeasurement) => this.timestampToDate(measurement.timestamp));
      const values = movementData.map((measurement: MovementMeasurement) => measurement.value);

      this.movementChart = new Chart('movementChart', {
        type: 'line',
        data: {
          labels: labels,
          datasets: [
            {
              label: 'Detected movement distance (cm)',
              data: values,
              pointBackgroundColor: 'black',
              pointRadius: 5,
              fill: false,
              showLine: false
            }
          ]
        },
        options: {
          scales: {
            y: {
              beginAtZero: true,
            }
          },
          responsive: true,
          maintainAspectRatio: false
        }
      });
    }
  }

  humidityChartWidth() {
    return ((this.measurements?.humidity.length || 0) * 50) + 'px';
  }

  temperatureChartWidth() {
    return ((this.measurements?.temperature.length || 0) * 100) + 'px';
  }

  movementChartWidth() {
    return ((this.measurements?.movement.length || 0) * 30) + 'px';
  }

  getLastHumidityDate(): string | null {
    if (this.measurements) {
      return this.timestampToDate(this.measurements.humidity[this.measurements.humidity.length - 1].timestamp);
    }

    return null;
  }

  getLastHumidity(): number | null {
    if (this.measurements?.temperature != null) {
      return this.measurements.humidity[this.measurements.humidity.length - 1].value;
    }

    return null;
  }

  getLastTemperatureDate(): string | null {
    if (this.measurements) {
      return this.timestampToDate(this.measurements.temperature[this.measurements.temperature.length - 1].timestamp);
    }

    return null;
  }

  getLastTemperature(): number | null {
    if (this.measurements?.temperature != null) {
      return this.measurements.temperature[this.measurements.temperature.length - 1].value;
    }

    return null;
  }

  getLastMovement(): number | null {
    if (this.measurements?.movement != null) {
      return this.measurements.movement[this.measurements.movement.length - 1].value;
    }

    return null;
  }

  getLastMovementDate(): string | null {
    if (this.measurements) {
      return this.timestampToDate(this.measurements.movement[this.measurements.movement.length - 1].timestamp);
    }

    return null;
  }
}
