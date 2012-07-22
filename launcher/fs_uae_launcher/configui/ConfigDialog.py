from __future__ import division
from __future__ import print_function
from __future__ import absolute_import

import fs_uae_launcher.fsui as fsui
from ..PagedDialog import PagedDialog
from ..I18N import _, ngettext
from .HardwarePage import HardwarePage
from .HardDrivesPage import HardDrivesPage
from .InputPage import InputPage
from .CustomOptionsPage import CustomOptionsPage

class ConfigDialog(PagedDialog):

    HARDWARE = 0
    INPUT = 1
    HARD_DRIVES = 2
    CUSTOM_OPTIONS = 3

    def __init__(self, parent, index):
        PagedDialog.__init__(self, parent, _("Configuration"))

        self.add_page(_("Hardware"), HardwarePage)
        self.add_page(_("Input"), InputPage)
        self.add_page(_("Hard Drives"), HardDrivesPage)
        self.add_page(_("Custom Options"), CustomOptionsPage)

        self.list_view.set_index(index)
        self.set_size((800, 540))
        self.center_on_parent()

    @classmethod
    def run(cls, parent, index):
        dialog = cls(parent, index)
        dialog.show_modal()
        dialog.destroy()
