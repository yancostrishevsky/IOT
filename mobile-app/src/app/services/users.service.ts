import { Injectable } from '@angular/core';
import { User } from '../interfaces/user';
import { AngularFireDatabase } from '@angular/fire/compat/database';
import { AngularFireAuth } from '@angular/fire/compat/auth';

@Injectable({
  providedIn: 'root'
})
export class UsersService {

  users: User[] = [];
  currentUser: User | null = null;

  constructor(private db: AngularFireDatabase, private afAuth: AngularFireAuth) {

    this.db.object('users').valueChanges().subscribe((data: any) => {

      if (data == null) {
        data = {};
      }

      this.users = [];

      Object.keys(data).forEach((key: any) => {

        const user = data[key];

        if (user.availableDevices == undefined) {
          user.availableDevices = [];
        }

        user.id = parseInt(key);

        this.users.push(user);
      });

      this.afAuth.authState.subscribe(user => {

        if (user == null) {
          this.currentUser = null;
          return;
        }

        const foundUser = this.users.find((u: User) => {
          return u.email == user.email;
        });

        this.currentUser = foundUser == undefined ? null : foundUser;
      });
    });


  }

  getCurrentUser(): User | null {
    return this.currentUser;
  }

  logout() {
    this.afAuth.signOut().then(() => {
      this.currentUser = null;
    });
  }

  signIn(email: string, password: string) {
    return this.afAuth.signInWithEmailAndPassword(email, password);
  }

  signUp(email: string, password: string) {
    return this.afAuth.createUserWithEmailAndPassword(email, password);
  }

  getNextId(): number {

    if (this.users.length == 0) {
      return 1;
    }

    return Math.max(...this.users.map((user: User) => user.id)) + 1;
  }

  createUser(email: string, nick: string) {

    const nextId = this.getNextId();

    this.db.object('users/' + nextId).set({
      email: email,
      nick: nick,
      availableDevices: []
    });
  }

  getUser(id: number): User | null {
    return this.users.find((user: User) => user.id == id) || null;
  }

  getUsers(): User[] {
    return this.users;
  }

  deleteOwnership(user: User, deviceName: string) {
    this.db.object('users/' + user.id + '/availableDevices').set(user.availableDevices.filter((device: string) => device != deviceName));
  }

  addOwnership(user: User | null, deviceName: string) {
    if (user == null) {
      return;
    }

    this.db.object('users/' + user.id + '/availableDevices').set([...user.availableDevices, deviceName]);
  }
}
