export interface HumidityMeasurement {
  value: number;
  timestamp: number;
};

export interface TemperatureMeasurement {
  value: number;
  timestamp: number;
};

export interface MovementMeasurement {
  timestamp: number;
  value: number;
}

export interface Measurements {
  humidity: HumidityMeasurement[];
  temperature: TemperatureMeasurement[];
  movement: MovementMeasurement[];
};
