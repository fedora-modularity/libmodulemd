#!/usr/bin/env python3

from babel.messages import Catalog, pofile
from babel._compat import StringIO, BytesIO
from datetime import datetime
from collections import defaultdict


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
        location = ("{}:{};description").format(
            stream.props.module_name, stream.props.stream_name)
        translation_dict[description].append(location)

        # Process summary
        summary = stream.get_summary("C")
        location = ("{}:{};summary").format(
            stream.props.module_name, stream.props.stream_name)
        translation_dict[summary].append(location)

        # Process profile descriptions(sometimes NULL)
        profile_names = stream.get_profile_names()
        if(profile_names):
            for pro_name in profile_names:
                profile = stream.get_profile(pro_name)

                profile_desc = profile.get_description("C")
                location = ("{}:{};profile;{}").format(
                    stream.props.module_name, stream.props.stream_name, pro_name)
                translation_dict[profile_desc].append(location)

    catalog = Catalog(project=project_name)

    for translatable_string, locations in translation_dict.items():
        catalog.add(translatable_string, locations=locations)

    return catalog
