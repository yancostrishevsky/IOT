import { Component } from '@angular/core';
import { RouterLink, RouterLinkActive } from '@angular/router';
import { UsersService } from '../../services/users.service';
import { NgIf } from '@angular/common';

@Component({
  selector: 'app-home',
  standalone: true,
  imports: [RouterLink, RouterLinkActive, NgIf],
  templateUrl: './home.component.html',
  styleUrl: './home.component.css'
})
export class HomeComponent {

  constructor(private usersService: UsersService) { }


  logout() {
    this.usersService.logout();
  }

  currentUser() {
    return this.usersService.getCurrentUser();
  }
}
