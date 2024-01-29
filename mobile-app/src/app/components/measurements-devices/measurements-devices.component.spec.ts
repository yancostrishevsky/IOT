import { ComponentFixture, TestBed } from '@angular/core/testing';

import { MeasurementsDevicesComponent } from './measurements-devices.component';

describe('MeasurementsDevicesComponent', () => {
  let component: MeasurementsDevicesComponent;
  let fixture: ComponentFixture<MeasurementsDevicesComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [MeasurementsDevicesComponent]
    })
    .compileComponents();
    
    fixture = TestBed.createComponent(MeasurementsDevicesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
