from __future__ import annotations

import os
import sys
import unreal
import json
from pathlib import Path
from typing import Dict, List

def fix_redirectors():
    unreal.log("Fix redirectors begin")
    asset_dir = "/Game/Environment/CommonAsset/Rocks/cave"
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    ar_filter = unreal.ARFilter(
        package_paths=[asset_dir],  # 查询路径
        recursive_paths=True,  # 是否递归查询子路径
        class_names=["ObjectRedirector"]
    )
    asset_registry.scan_paths_synchronous([asset_dir])
    asset_data_list = asset_registry.get_assets(ar_filter)
    unreal.log(f"Found {len(asset_data_list)} redirectors")

    redirectors_to_fix = []
    # 输出资产信息
    for asset_data in asset_data_list:
        asset = unreal.load_asset(asset_data.object_path, follow_redirectors=False)
        if asset:
            unreal.log(f"Load asset {asset.get_full_name()} succeed")
            redirectors_to_fix.append(asset)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    unreal.log(dir(asset_tools))

    who_checked_out_map = asset_tools.pretest_for_fixup_referencers(redirectors_to_fix)

    if len(who_checked_out_map) == 0:
        unreal.log("Pretest pass")
    else:
        unreal.log(f"Pretest not pass")
        for k, v in who_checked_out_map.items():
            unreal.log(f"WhoCheckedOut: {k} checked out by {v}")

if __name__ == "__main__":
    unreal.log_warning("asset_tools.py")
    fix_redirectors()
