import { Routes } from '@angular/router';
import { HomeComponent } from './components/home/home.component';
import { MeasurementsDevicesComponent } from './components/measurements-devices/measurements-devices.component';
import { ConfigureDevicesComponent } from './components/configure-devices/configure-devices.component';
import { SignInComponent } from './components/sign-in/sign-in.component';
import { SignUpComponent } from './components/sign-up/sign-up.component';

export const routes: Routes = [
  { path: '', redirectTo: 'home', pathMatch: 'full' },
  { path: 'home', component: HomeComponent },
  { path: 'measurements-devices', component: MeasurementsDevicesComponent },
  { path: 'configure-devices', component: ConfigureDevicesComponent },
  { path: 'sign-in', component: SignInComponent },
  { path: 'sign-up', component: SignUpComponent }
];
