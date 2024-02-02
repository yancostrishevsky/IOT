import { CapacitorConfig } from '@capacitor/cli';

const config: CapacitorConfig = {
  appId: 'com.example.app',
  appName: 'server-room-monitor',
  webDir: 'dist/mobile-app/browser',
  server: {
    androidScheme: 'https'
  }
};

export default config;
