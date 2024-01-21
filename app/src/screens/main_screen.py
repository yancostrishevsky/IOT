from kivy.uix.screenmanager import Screen
from kivy.properties import ObjectProperty


class MainScreen(Screen):
    header = ObjectProperty()
    main_layout = ObjectProperty()
    icon = ObjectProperty()
    flashcards_btn = ObjectProperty()
    revising_btn = ObjectProperty()
    info_help_btn = ObjectProperty()

    def __init__(self, **kwargs):
        super(MainScreen, self).__init__(**kwargs)