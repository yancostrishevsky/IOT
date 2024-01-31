import { Component } from '@angular/core';
import { FormControl, FormGroup, ReactiveFormsModule, Validators } from '@angular/forms';
import { Router, RouterLink, RouterLinkActive } from '@angular/router';
import { NgIf } from '@angular/common';
import { UsersService } from '../../services/users.service';

@Component({
  selector: 'app-signin',
  standalone: true,
  imports: [ReactiveFormsModule, RouterLink, RouterLinkActive, NgIf],
  templateUrl: './sign-in.component.html',
  styleUrl: './sign-in.component.css'
})
export class SignInComponent {

  formGroup: FormGroup;

  errorMessage: string = '';

  constructor(private usersService: UsersService, private router: Router) {

    this.formGroup = new FormGroup({
      email: new FormControl('', [Validators.required]),
      password: new FormControl('', [Validators.required])
    });

  }

  login() {

    this.errorMessage = '';

    this.usersService.signIn(this.formGroup.value.email, this.formGroup.value.password).catch((error) => {
      this.errorMessage = 'Invalid email or password.';
    }).then((data) => {
      this.formGroup.reset();
      this.router.navigate(['/home']);
    });
  }

}
