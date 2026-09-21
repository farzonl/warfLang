# Copyright (c) 2026 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

import os

import lit.formats

config.name = "warf"
config.test_format = lit.formats.ShTest()
config.suffixes = [".wf", ".wir"]
config.excludes = ["Inputs"]
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = config.test_source_root

site_config = os.environ.get("WARF_LIT_SITE_CONFIG")
if site_config:
    lit_config.load_config(config, site_config)
