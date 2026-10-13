#!/usr/bin/env python3
"""Build the GRASPion SAMD Arduino Boards Manager package.

The package is based on Adafruit ArduinoCore-samd 1.7.17. It keeps the
Adafruit SAMD core/toolchain support, but exposes only the Adafruit QT Py M0
and the GRASPion Body board.

ArduinoCore-samd 1.7.17 uses Adafruit_TinyUSB_Arduino and Adafruit_ZeroDMA as
git submodules. GitHub source archives do not contain submodule contents, so
this builder downloads the exact submodule commits referenced by the core.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import tarfile
import tempfile
import urllib.request
from pathlib import Path

ADAFRUIT_CORE_VERSION = "1.7.17"
ADAFRUIT_CORE_URL = (
    "https://github.com/adafruit/ArduinoCore-samd/"
    f"archive/refs/tags/{ADAFRUIT_CORE_VERSION}.tar.gz"
)
ADAFRUIT_INDEX_URL = (
    "https://adafruit.github.io/arduino-board-index/package_adafruit_index.json"
)

TINYUSB_COMMIT = "c953968c468218d8968138b5ed8482fa565373df"
ZERODMA_COMMIT = "acc5dadb458b2c329757a61dc4f18dda945a0c36"

SUBMODULES = {
    "Adafruit_TinyUSB_Arduino": {
        "repo": "adafruit/Adafruit_TinyUSB_Arduino",
        "commit": TINYUSB_COMMIT,
        "required": "src/Adafruit_TinyUSB.h",
    },
    "Adafruit_ZeroDMA": {
        "repo": "adafruit/Adafruit_ZeroDMA",
        "commit": ZERODMA_COMMIT,
        "required": "Adafruit_ZeroDMA.h",
    },
}


def download(url: str, destination: Path) -> None:
    print(f"Downloading {url}")
    with urllib.request.urlopen(url) as response, destination.open("wb") as out:
        shutil.copyfileobj(response, out)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def extract_archive(archive: Path, destination: Path) -> Path:
    with tarfile.open(archive, "r:gz") as tar:
        members = tar.getmembers()
        if not members:
            raise RuntimeError(f"Empty archive: {archive}")
        root_name = members[0].name.split("/", 1)[0]
        tar.extractall(destination, filter="data")
    root = destination / root_name
    if not root.exists():
        raise RuntimeError(f"Could not find extracted root {root}")
    return root


def install_submodule(package_root: Path, tmp: Path, name: str, spec: dict) -> None:
    archive = tmp / f"{name}.tar.gz"
    url = f"https://github.com/{spec['repo']}/archive/{spec['commit']}.tar.gz"
    download(url, archive)
    extracted = extract_archive(archive, tmp)

    destination = package_root / "libraries" / name
    if destination.exists():
        shutil.rmtree(destination)
    shutil.copytree(extracted, destination)

    required = destination / spec["required"]
    if not required.exists():
        raise RuntimeError(f"{name} is incomplete: missing {required}")


def validate_package(package_root: Path) -> None:
    required_paths = [
        package_root / "libraries" / "Adafruit_ZeroDMA" / "Adafruit_ZeroDMA.h",
        package_root
        / "libraries"
        / "Adafruit_TinyUSB_Arduino"
        / "src"
        / "Adafruit_TinyUSB.h",
        package_root / "variants" / "qtpy_m0" / "variant.h",
        package_root / "variants" / "graspionBody" / "variant.h",
        package_root / "bootloaders" / "qtpyM0" / "bootloader-qtpy_m0.bin",
    ]
    missing = [str(path) for path in required_paths if not path.exists()]
    if missing:
        raise RuntimeError("Incomplete package:\n  " + "\n  ".join(missing))


def find_adafruit_samd_platform(index: dict) -> tuple[dict, dict]:
    for package in index["packages"]:
        if package.get("name") != "adafruit":
            continue
        for platform in package.get("platforms", []):
            if (
                platform.get("architecture") == "samd"
                and platform.get("version") == ADAFRUIT_CORE_VERSION
            ):
                return package, platform
    raise RuntimeError(
        f"Adafruit SAMD platform {ADAFRUIT_CORE_VERSION} not found in package index"
    )


def merge_tools(existing: list[dict], incoming: list[dict]) -> list[dict]:
    merged: dict[tuple[str, str], dict] = {}
    for tool in existing + incoming:
        key = (tool.get("name", ""), tool.get("version", ""))
        merged[key] = tool
    return list(merged.values())


def merge_index(existing_index: dict | None, platform: dict, tools: list[dict]) -> dict:
    existing_platforms: list[dict] = []
    existing_tools: list[dict] = []

    if existing_index:
        for package in existing_index.get("packages", []):
            if package.get("name") == "graspion":
                existing_platforms = [
                    p
                    for p in package.get("platforms", [])
                    if p.get("architecture") != "samd"
                ]
                existing_tools = package.get("tools", [])
                break

    return {
        "packages": [
            {
                "name": "graspion",
                "maintainer": "GRASP Lab - University of Liège",
                "websiteURL": "https://graspion.be",
                "platforms": existing_platforms + [platform],
                "tools": merge_tools(existing_tools, tools),
            }
        ]
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--version", default="1.0.0", help="GRASPion SAMD package version")
    parser.add_argument(
        "--repo",
        default="GRASP-LAB/GRASPion",
        help="GitHub repository used for release URLs",
    )
    parser.add_argument(
        "--output", default="dist", help="Output directory relative to repository root"
    )
    args = parser.parse_args()

    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent.parent
    output_dir = repo_root / args.output
    output_dir.mkdir(parents=True, exist_ok=True)

    board_fragment = script_dir / "samd.boards.txt"
    graspion_variant_dir = script_dir / "variants" / "graspionBody"

    if not board_fragment.exists():
        raise FileNotFoundError(board_fragment)
    if not graspion_variant_dir.exists():
        raise FileNotFoundError(graspion_variant_dir)

    package_root_name = f"graspion-samd-{args.version}"
    archive_name = f"{package_root_name}.tar.bz2"
    archive_path = output_dir / archive_name
    release_tag = f"arduino-samd-{args.version}"
    release_url = (
        f"https://github.com/{args.repo}/releases/download/"
        f"{release_tag}/{archive_name}"
    )

    with tempfile.TemporaryDirectory() as tmp_name:
        tmp = Path(tmp_name)
        source_archive = tmp / "adafruit-samd.tar.gz"
        download(ADAFRUIT_CORE_URL, source_archive)

        extracted = extract_archive(source_archive, tmp)
        package_root = tmp / package_root_name
        shutil.copytree(extracted, package_root)

        # GitHub source archives omit submodule contents. Install the exact
        # submodule revisions pinned by ArduinoCore-samd 1.7.17.
        for name, spec in SUBMODULES.items():
            install_submodule(package_root, tmp, name, spec)

        board_text = board_fragment.read_text(encoding="utf-8")
        if not board_text.endswith("\n"):
            board_text += "\n"
        (package_root / "boards.txt").write_text(board_text, encoding="utf-8")

        variants_root = package_root / "variants"
        qtpy_source = variants_root / "qtpy_m0"
        if not qtpy_source.exists():
            raise FileNotFoundError(qtpy_source)

        saved_qtpy = tmp / "qtpy_m0"
        shutil.copytree(qtpy_source, saved_qtpy)
        shutil.rmtree(variants_root)
        variants_root.mkdir(parents=True)

        # Keep the original QT Py M0 variant.
        shutil.copytree(saved_qtpy, variants_root / "qtpy_m0")

        # Start GRASPion Body from the complete upstream QT Py M0 variant so
        # linker/debug/openocd support remains exactly aligned with Adafruit.
        graspion_destination = variants_root / "graspionBody"
        shutil.copytree(saved_qtpy, graspion_destination)
        for filename in ("variant.h", "variant.cpp", "pins_arduino.h"):
            source = graspion_variant_dir / filename
            if not source.exists():
                raise FileNotFoundError(source)
            shutil.copy2(source, graspion_destination / filename)

        validate_package(package_root)

        with tarfile.open(archive_path, "w:bz2") as tar:
            tar.add(package_root, arcname=package_root_name)

    checksum = sha256(archive_path)
    archive_size = archive_path.stat().st_size

    with urllib.request.urlopen(ADAFRUIT_INDEX_URL) as response:
        adafruit_index = json.load(response)

    adafruit_package, adafruit_platform = find_adafruit_samd_platform(adafruit_index)

    tools = adafruit_package.get("tools", [])
    tools_dependencies = []
    for dependency in adafruit_platform.get("toolsDependencies", []):
        dep = dict(dependency)
        if dep.get("packager") == "adafruit":
            dep["packager"] = "graspion"
        tools_dependencies.append(dep)

    platform = {
        "name": "GRASPion SAMD Boards",
        "architecture": "samd",
        "version": args.version,
        "category": "Contributed",
        "url": release_url,
        "archiveFileName": archive_name,
        "checksum": f"SHA-256:{checksum}",
        "size": str(archive_size),
        "boards": [
            {"name": "Adafruit QT Py M0 (SAMD21)"},
            {"name": "GRASPion Body (SAMD21)"},
        ],
        "toolsDependencies": tools_dependencies,
    }

    existing_index_path = repo_root / "package_graspion_index.json"
    existing_index = None
    if existing_index_path.exists():
        existing_index = json.loads(existing_index_path.read_text(encoding="utf-8"))

    result = merge_index(existing_index, platform, tools)

    index_path = output_dir / "package_graspion_index.json"
    index_path.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")

    print(f"Created {archive_path}")
    print(f"SHA-256: {checksum}")
    print(f"Size: {archive_size}")
    print(f"Created {index_path}")


if __name__ == "__main__":
    main()
