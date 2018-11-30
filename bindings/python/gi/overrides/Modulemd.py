#!/usr/bin/python3

# This file is part of libmodulemd
# Copyright (C) 2016 Red Hat, Inc.
# Copyright (C) 2017-2018 Stephen Gallagher
#
# Fedora-License-Identifier: MIT
# SPDX-2.0-License-Identifier: MIT
# SPDX-3.0-License-Identifier: MIT
#
# This program is free software.
# For more information on the license, see COPYING.
# For more information on free software, see
# <https://www.gnu.org/philosophy/free-sw.en.html>.#!/usr/bin/python3

# This file is part of libmodulemd
# Copyright (C) 2017-2018 Stephen Gallagher
#
# Fedora-License-Identifier: MIT
# SPDX-2.0-License-Identifier: MIT
# SPDX-3.0-License-Identifier: MIT
#
# This program is free software.
# For more information on the license, see COPYING.
# For more information on free software, see
# <https://www.gnu.org/philosophy/free-sw.en.html>.


import gi

from ..module import get_introspection_module
from ..overrides import override

Modulemd = get_introspection_module('Modulemd')

from six import text_type
from gi.repository import GLib

__all__ = []

class ModulemdUtil():
    def variant_str(s):
        """ Converts a string to a GLib.Variant
        """
        if not isinstance(s, str):
            raise TypeError('Only strings are supported for scalars')

        return GLib.Variant('s', s)

    def variant_bool(b):
        """ Converts a boolean to a GLib.Varant
        """
        if not isinstance(b, bool):
            raise TypeError('Only booleans are supported')

        return GLib.Variant('b', b)

    def variant_list(l):
        """ Converts a list to a GLib.Variant
        """
        l_variant = list()
        for item in l:
            if item is None:
                item = ''

            l_variant.append(ModulemdUtil.python_to_variant(item))
        return GLib.Variant('av', l_variant)

    def variant_dict(d):
        """ Converts a dictionary to a dictionary of GLib.Variant
        """
        if not isinstance(d, dict):
            raise TypeError('Only dictionaries are supported for mappings')

        d_variant = ModulemdUtil.dict_values(d)
        return GLib.Variant('a{sv}', d_variant)

    def dict_values(d):
        """ Converts each dictionary value to a GLib.Variant
        """
        if not isinstance(d, dict):
            raise TypeError('Only dictionaries are supported for mappings')

        d_variant = dict()
        for k, v in d.items():
            if v is None:
                v = ''

            d_variant[k] = ModulemdUtil.python_to_variant(v)
        return d_variant

    def python_to_variant(obj):
        if isinstance(obj, str):
            return ModulemdUtil.variant_str(obj)

        elif isinstance(obj, text_type):
            return ModulemdUtil.variant_str(obj.encode('utf-8'))

        elif isinstance(obj, bool):
            return ModulemdUtil.variant_bool(obj)

        elif isinstance(obj, list):
            return ModulemdUtil.variant_list(obj)

        elif isinstance(obj, dict):
            return ModulemdUtil.variant_dict(obj)

        else:
            raise TypeError('Cannot convert unknown type')


if "ModuleStreamV2" in dir(Modulemd):
    class ModuleStreamV2(Modulemd.ModuleStreamV2):

        def set_xmd(self, xmd):
            super().set_xmd(ModulemdUtil.python_to_variant(xmd))

        def get_xmd(self):
            variant_xmd = super().get_xmd()
            return variant_xmd.unpack()


    ModuleStreamV2 = override(ModuleStreamV2)
    __all__.append(ModuleStreamV2)


if "ModuleStreamV1" in dir(Modulemd):
    class ModuleStreamV1(Modulemd.ModuleStreamV1):

        def set_xmd(self, xmd):
            super().set_xmd(ModulemdUtil.python_to_variant(xmd))

        def get_xmd(self):
            variant_xmd = super().get_xmd()
            return variant_xmd.unpack()


    ModuleStreamV1 = override(ModuleStreamV1)
    __all__.append(ModuleStreamV1)

