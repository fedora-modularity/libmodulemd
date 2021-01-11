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
# <https://www.gnu.org/philosophy/free-sw.en.html>.

from ..module import get_introspection_module
from ..overrides import override

import functools

from six import text_type
from gi.repository import GLib

import datetime

Modulemd = get_introspection_module("Modulemd")


__all__ = []


class ModulemdUtil(object):
    @staticmethod
    def strip_gtype(method):
        @functools.wraps(method)
        def wrapped(*args, **kwargs):
            ret = method(*args, **kwargs)
            if len(ret) == 2:
                return ret[1]
            else:
                return ret[1:]

        return wrapped

    @staticmethod
    def variant_str(s):
        """Converts a string to a GLib.Variant"""
        if not isinstance(s, str):
            raise TypeError("Only strings are supported for scalars")

        return GLib.Variant.new_string(s)

    @staticmethod
    def variant_bool(b):
        """Converts a boolean to a GLib.Varant"""
        if not isinstance(b, bool):
            raise TypeError("Only booleans are supported")

        return GLib.Variant.new_boolean(b)

    @staticmethod
    def variant_list(vl):
        """Converts a list to a GLib.Variant"""

        # If this is a zero-length array, handle it specially
        if len(vl) == 0:
            return GLib.Variant.new_array(GLib.VariantType("v"))

        # Build the array from each entry
        builder = GLib.VariantBuilder(GLib.VariantType("a*"))
        for item in vl:
            if item is None:
                item = ""
            builder.add_value(ModulemdUtil.python_to_variant(item))

        return builder.end()

    @staticmethod
    def variant_dict(d):
        """Converts a dictionary to a dictionary of GLib.Variant"""
        if not isinstance(d, dict):
            raise TypeError("Only dictionaries are supported for mappings")

        vdict = GLib.VariantDict()

        for k, v in d.items():
            if v is None:
                v = ""
            vdict.insert_value(k, ModulemdUtil.python_to_variant(v))

        return vdict.end()

    @staticmethod
    def python_to_variant(obj):

        if isinstance(obj, str):
            return ModulemdUtil.variant_str(obj)

        elif isinstance(obj, text_type):
            return ModulemdUtil.variant_str(obj.encode("utf-8"))

        elif isinstance(obj, bool):
            return ModulemdUtil.variant_bool(obj)

        elif isinstance(obj, list):
            return ModulemdUtil.variant_list(obj)

        elif isinstance(obj, dict):
            return ModulemdUtil.variant_dict(obj)

        else:
            raise TypeError("Cannot convert unknown type")


if float(Modulemd._version) >= 2:

    if hasattr(Modulemd, "read_packager_file"):
        read_packager_file = ModulemdUtil.strip_gtype(
            Modulemd.read_packager_file
        )
        __all__.append("read_packager_file")

    if hasattr(Modulemd, "read_packager_string"):
        read_packager_string = ModulemdUtil.strip_gtype(
            Modulemd.read_packager_string
        )
        __all__.append("read_packager_string")

    if hasattr(Modulemd.ModuleIndex, "add_known_stream"):

        class ModuleIndex(Modulemd.ModuleIndex):
            def set_known_streams(self, known_streams):
                for module, streams in known_streams.items():
                    for stream in streams:
                        super(ModuleIndex, self).add_known_stream(
                            module, stream
                        )

        ModuleIndex = override(ModuleIndex)
        __all__.append(ModuleIndex)

    if hasattr(Modulemd, "PackagerV3"):

        class PackagerV3(Modulemd.PackagerV3):
            def set_xmd(self, xmd):
                super(PackagerV3, self).set_xmd(
                    ModulemdUtil.python_to_variant(xmd)
                )

            def get_xmd(self):
                variant_xmd = super(PackagerV3, self).get_xmd()
                if variant_xmd is None:
                    return {}
                return variant_xmd.unpack()

        PackagerV3 = override(PackagerV3)
        __all__.append(PackagerV3)

    if hasattr(Modulemd, "ModuleStreamV2"):

        class ModuleStreamV2(Modulemd.ModuleStreamV2):
            def set_xmd(self, xmd):
                super(ModuleStreamV2, self).set_xmd(
                    ModulemdUtil.python_to_variant(xmd)
                )

            def get_xmd(self):
                variant_xmd = super(ModuleStreamV2, self).get_xmd()
                if variant_xmd is None:
                    return {}
                return variant_xmd.unpack()

        ModuleStreamV2 = override(ModuleStreamV2)
        __all__.append(ModuleStreamV2)

    if hasattr(Modulemd, "ModuleStreamV1"):

        class ModuleStreamV1(Modulemd.ModuleStreamV1):
            def set_xmd(self, xmd):
                super(ModuleStreamV1, self).set_xmd(
                    ModulemdUtil.python_to_variant(xmd)
                )

            def get_xmd(self):
                variant_xmd = super(ModuleStreamV1, self).get_xmd()
                if variant_xmd is None:
                    return {}
                return variant_xmd.unpack()

        ModuleStreamV1 = override(ModuleStreamV1)
        __all__.append(ModuleStreamV1)

    if hasattr(Modulemd, "ServiceLevel"):

        class ServiceLevel(Modulemd.ServiceLevel):
            def set_eol(self, eol):
                if isinstance(eol, datetime.date):
                    return super(ServiceLevel, self).set_eol_ymd(
                        eol.year, eol.month, eol.day
                    )

                raise TypeError(
                    "Expected datetime.date, but got %s."
                    % (type(eol).__name__)
                )

            def get_eol(self):
                eol = super(ServiceLevel, self).get_eol()
                if eol is None:
                    return None

                return datetime.date(
                    eol.get_year(), eol.get_month(), eol.get_day()
                )

        ServiceLevel = override(ServiceLevel)
        __all__.append(ServiceLevel)
