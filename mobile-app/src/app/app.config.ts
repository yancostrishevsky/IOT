import { ApplicationConfig, importProvidersFrom } from '@angular/core';
import { provideRouter } from '@angular/router';

import { routes } from './app.routes';
import { initializeApp, provideFirebaseApp } from '@angular/fire/app';
import { getAuth, provideAuth } from '@angular/fire/auth';
import { getDatabase, provideDatabase } from '@angular/fire/database';
import { environment } from '../environments/environment.prod';
import {FIREBASE_OPTIONS} from '@angular/fire/compat';

export const appConfig: ApplicationConfig = {
  providers: [provideRouter(routes), importProvidersFrom(provideFirebaseApp(() => initializeApp(environment.firebaseConfig))),
    importProvidersFrom(provideAuth(() => getAuth())),
    importProvidersFrom(provideDatabase(() => getDatabase())),

    { provide: FIREBASE_OPTIONS, useValue: environment.firebaseConfig }]
};
