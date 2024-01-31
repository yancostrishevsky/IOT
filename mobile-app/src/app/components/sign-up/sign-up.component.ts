import { Component } from '@angular/core';
import { FormControl, FormGroup, ReactiveFormsModule, Validators } from '@angular/forms';
import { Router, RouterLink, RouterLinkActive } from '@angular/router';
import { UsersService } from '../../services/users.service';

@Component({
  selector: 'app-signup',
  standalone: true,
  imports: [ReactiveFormsModule, RouterLink, RouterLinkActive],
  templateUrl: './sign-up.component.html',
  styleUrl: './sign-up.component.css'
})
export class SignUpComponent {
  formGroup: FormGroup;

  errorMessage: string = '';

  constructor(private usersService: UsersService, private router: Router) {

    this.formGroup = new FormGroup({
      email: new FormControl('', [Validators.required, Validators.email]),
      password: new FormControl('', [Validators.required, Validators.minLength(6)]),
      nick: new FormControl('', [Validators.required, Validators.minLength(3)])
    });

  }

  register() {

    this.errorMessage = '';

    this.usersService.signUp(this.formGroup.value.email, this.formGroup.value.password).then(data => {
      this.usersService.createUser(this.formGroup.value.email, this.formGroup.value.nick);
      this.formGroup.reset();
      this.router.navigate(['/home']);
    }).catch((error: any) => {
      this.errorMessage = 'Failed to sign up!';
    });


  }
}
