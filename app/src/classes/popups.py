from kivy.uix.popup import Popup
from kivy.uix.button import Button
from kivy.utils import get_color_from_hex
from kivy.cache import Cache
from kivy.uix.image import Image
from kivy.metrics import dp
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.scrollview import ScrollView
from kivy.effects.scroll import ScrollEffect
from kivy.uix.label import Label
from kivy.uix.floatlayout import FloatLayout


def ok_popup(title: str, screen_width: float, height_hint=0.45, btn_callback=None):
    workdir = Cache.get("app_info", "work_dir")
    popup_text = title

    btn = Button(text="ok", color=get_color_from_hex("#444444"),
                 font_size=screen_width * 0.8 * 0.12 * 0.6,
                 background_normal=workdir + "/data/textures/yes_button_normal.png",
                 background_down=workdir + "/data/textures/yes_button_down.png", opacity=0.7)
    ok_pop = Popup(title=popup_text,
                   auto_dismiss=False,
                   size_hint=(0.9, None), title_align="center", separator_color=(0, 0, 0, 0),
                   title_color=get_color_from_hex("#444444"),
                   background=workdir + "/data/textures/popup_background.png",
                   title_size=screen_width * 0.8 * 0.12 * 0.6, height=screen_width * height_hint,
                   content=btn, border=[0, 0, 0, 0])
    btn.bind(on_release=ok_pop.dismiss)

    if btn_callback is not None:
        btn.bind(on_release=btn_callback)

    return ok_pop

def yes_no_popup(title: str, screen_width: float, height_hint=0.5, yes_callback=None, no_callback=None):
    workdir = Cache.get("app_info", "work_dir")

    pop_content = BoxLayout(orientation="horizontal", spacing=dp(10))
    yes_btn = Button(text="yes", color=get_color_from_hex("#444444"),
                     font_size=screen_width * 0.1 * 0.6,
                     background_normal=workdir + "/data/textures/yes_button_normal.png",
                     background_down=workdir + "/data/textures/yes_button_down.png", opacity=0.7)
    no_btn = Button(text="no", color=get_color_from_hex("#444444"),
                    font_size=screen_width * 0.1 * 0.6,
                    background_normal=workdir + "/data/textures/no_button_normal.png",
                    background_down=workdir + "/data/textures/no_button_down.png", opacity=0.7)
    pop_content.add_widget(yes_btn)
    pop_content.add_widget(no_btn)

    workdir = Cache.get("app_info", "work_dir")
    info_pop = Popup(title=title, content=pop_content, auto_dismiss=True,
                     size_hint=(0.9, None), title_align="center", separator_color=(0, 0, 0, 0),
                     title_color=get_color_from_hex("#444444"),
                     background=workdir + "/data/textures/popup_background.png",
                     title_size=screen_width * 0.1 * 0.6, height=screen_width * height_hint,
                     border=[0, 0, 0, 0])

    if yes_callback is not None:
        yes_btn.bind(on_release=yes_callback)

    if no_callback is not None:
        no_btn.bind(on_release=no_callback)

    yes_btn.bind(on_release=info_pop.dismiss)
    no_btn.bind(on_release=info_pop.dismiss)

    return info_pop
