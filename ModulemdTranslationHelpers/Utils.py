import sys
from babel.messages import Catalog, pofile
from datetime import datetime
from collections import defaultdict

try:
    import gi
    gi.require_version('Modulemd', '2.0')
    from gi.repository import Modulemd
except ImportError:
    sys.exit(77)


def get_translation_catalog_from_index(index, project_name):
    # Get all Modulemd.Module object names
    module_names = index.get_module_names()

    # Create a list containing highest version streams of a module
    final_streams = list()

    for module_name in module_names:
        module = index.get_module(module_name)
        stream_names = module.get_stream_names()

        for stream_name in stream_names:
            # The first item returned is guaranteed to be the highest version
            # of that stream in that module.
            stream = module.search_streams(stream_name, 0)[0]
            final_streams.append(stream)

    # A dictionary to store:
    # key: all translatable strings
    # value: their respective locations
    translation_dict = defaultdict(list)

    for stream in final_streams:
        # Process description
        description = stream.get_description("C")
        location = ("{};{};description").format(
            stream.props.module_name, stream.props.stream_name)
        translation_dict[description].append(location)

        # Process summary
        summary = stream.get_summary("C")
        location = ("{};{};summary").format(
            stream.props.module_name, stream.props.stream_name)
        translation_dict[summary].append(location)

        # Process profile descriptions(sometimes NULL)
        profile_names = stream.get_profile_names()
        if(profile_names):
            for pro_name in profile_names:
                profile = stream.get_profile(pro_name)

                profile_desc = profile.get_description("C")
                location = ("{};{};profile;{}").format(
                    stream.props.module_name, stream.props.stream_name, pro_name)
                translation_dict[profile_desc].append(location)

    catalog = Catalog(project=project_name)

    for translatable_string, locations in translation_dict.items():
        catalog.add(translatable_string, locations=locations)

    return catalog


def get_modulemd_translations_from_catalog(catalogs, index):
    # Dictionary `translations` contains information from catalog like:
    # Key: (module_name, stream_name)
    # Value: Translation object containing TranslationEntry of various locales
    translations = dict()

    now = datetime.utcnow()
    modified = int(now.strftime("%Y%m%d%H%M%S"))

    # Handling one language translations at a time
    for catalog in catalogs:
        # Dictionary `data` contains information from catalog like:
        # Key: (module_name, stream_name)
        # Value: TranslationEntry object of a locale
        data = dict()

        for msg in catalog:
            for location, _ in msg.locations:
                (module_name, stream_name, string_type,
                 profile_name) = split_location(location)

                try:
                    entry = data[(module_name, stream_name)]
                except KeyError:
                    entry = Modulemd.TranslationEntry.new(
                        str(catalog.locale))

                if string_type == "summary":
                    entry.set_summary(msg.string)
                elif string_type == "description":
                    entry.set_description(msg.string)
                else:
                    entry.set_profile_description(profile_name, msg.string)

                data[(module_name, stream_name)] = entry

        for (module_name, stream_name), entry in data.items():
            try:
                mmd_translation = translations[(module_name, stream_name)]
            except KeyError:
                mmd_translation = Modulemd.Translation.new(
                    1, module_name, stream_name, modified)

            mmd_translation.set_translation_entry(entry)
            translations[(module_name, stream_name)] = mmd_translation

    for (module_name, stream_name), mmd_translation in translations.items():
        try:
            ret = index.add_translation(mmd_translation)
        except GLib.Error:
            print(
                "The translation for this %s:%s could not be added",
                module_name,
                stream_name)

    return None


def split_location(location):
    # Split maximum three times
    props = location.split(";", 3)

    module_name = props[0]
    stream_name = props[1]
    string_type = props[2]

    if string_type == "profile":
        profile_name = props[3]
    else:
        profile_name = None

    return module_name, stream_name, string_type, profile_name
