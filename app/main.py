from kivy.app import App
from kivy.config import Config
from os import curdir, path

Config.read("data/config/ServerRoomMonitor.ini")

from kivy import lang
from kivy.uix import screenmanager
from kivy import cache
from kivy import utils
from kivy.core import window
import kivy

from src.screens import main_screen
from src.screens import sensor_choice_screen
from src.screens import config_screen

import ctypes

kivy.require("2.1.0")

from kivy import factory

factory.Factory.register("MenuButton", module="src.classes.menu_btn")


class WindowManager(screenmanager.ScreenManager):

    def blur(self, transition, progression):
        transition.screen_in.opacity = progression
        transition.screen_out.opacity = 1 - progression

    def on_current(self, instance, value):
        self.old = self.new
        self.new = value
        super(WindowManager, self).on_current(instance, value)

    def __init__(self, **kwargs):
        super(WindowManager, self).__init__(**kwargs)
        self.transition.bind(on_progress=self.blur)

        # screen names to handle returning to previous screen
        self.new = None
        self.old = None


class ServerRoomMonitorApp(App):

    def switch_screen(self, screen_name, direction):
        # var screen_name has to store the name of the screen we want to switch to
        # the name is always name of the instance of the screen lowercase, eg. MAIN_S -> main_s
        direction_map = {
            "up": "down",
            "down": "up",
            "left": "right",
            "right": "left"
        }

        if direction == "inverted":
            self.window_manager.transition.direction = direction_map[self.window_manager.transition.direction]
        else:
            self.window_manager.transition.direction = direction

        if screen_name in self.window_manager.screen_names:
            self.window_manager.current = screen_name
        elif screen_name == "previous":
            self.window_manager.current = self.window_manager.old

    def on_pause(self):
        return True

    def on_start(self):
        pass

    def on_stop(self):
        pass

    def go_back(self, window, key, *args):
        if key == 27 and hasattr(self.window_manager.current_screen, "back_btn"):
            self.window_manager.current_screen.back_btn.dispatch("on_release")

    def build_config(self, config):
        # registering all important cache's
        cache.Cache.register("app_info")  # platform, work_dir, database_dir, aux_database_dir, rootpath
        # ...

        # saving values to app_info
        cache.Cache.append("app_info", "platform", utils.platform)
        cache.Cache.append("app_info", "work_dir", path.abspath(curdir))

        if cache.Cache.get("app_info", "platform") == "win":
            user32 = ctypes.windll.user32
            s_width, s_height = user32.GetSystemMetrics(78), user32.GetSystemMetrics(79)

            Config.set("graphics", "width", int(s_width * 0.9))
            Config.set("graphics", "height", int(s_width * 0.9))
            cache.Cache.append("app_info", "rootpath", "C:/Users")

            # ///// for developer /////////
            Config.set("graphics", "width", 380)
            Config.set("graphics", "height", 736)
            # /////////////////////////////

            # Config.write()

        else:
            cache.Cache.append("app_info", "rootpath", "/storage/emulated/0")

    def build(self):

        # loading all kv files
        screen_names = ["main_screen", "sensor_choice_screen", "config_screen"]  # ...

        path_to_kv = cache.Cache.get("app_info", "work_dir") + "/data/kv/"
        kv_filenames = [path_to_kv + name + ".kv" for name in screen_names]

        lang.Builder.load_file(cache.Cache.get("app_info", "work_dir") + "/data/kv/main_layout.kv")

        for filename in kv_filenames:
            lang.Builder.load_file(filename)
        # ...

        # creating instances of all screens and adding to window manager
        screen_list = [main_screen.MainScreen(name="main_screen"),
                       sensor_choice_screen.SensorChoiceScreen(name="sensor_choice_screen"),
                       config_screen.ConfigScreen(name="config_screen")]

        self.window_manager = WindowManager(transition=screenmanager.SlideTransition())

        for screen in screen_list:
            self.window_manager.add_widget(screen)
        # ...

        # /// dev /////////////////////////////////////////
        # self.window_manager.current = "add_from_dictionary_screen"
        # ///////////////////////////////////////////////

        window.Window.bind(on_keyboard=self.go_back)

        self.icon = cache.Cache.get("app_info", "work_dir") + "/logo.png"
        self.title = "ServerRoomMonitor"
        return self.window_manager

    def __init__(self, **kwargs):
        super(ServerRoomMonitorApp, self).__init__(**kwargs)
        self.window_manager = None  # instance is created in build method so that kv rules can be aplied to window manager


if __name__ == "__main__":
    ServerRoomMonitorApp().run()
