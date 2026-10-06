#!/usr/bin/env python3
"""Build the GRASPion nRF52 Arduino Boards Manager package.

The package is based on Adafruit_nRF52_Arduino 1.7.0. It keeps the
Adafruit core and toolchain support, but exposes only the GRASPion Head
board and its dedicated variant.
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

ADAFRUIT_CORE_VERSION = "1.7.0"
ADAFRUIT_CORE_URL = (
    "https://github.com/adafruit/Adafruit_nRF52_Arduino/"
    f"archive/refs/tags/{ADAFRUIT_CORE_VERSION}.tar.gz"
)
ADAFRUIT_INDEX_URL = (
    "https://adafruit.github.io/arduino-board-index/package_adafruit_index.json"
)


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


def find_adafruit_nrf52_platform(index: dict) -> tuple[dict, dict]:
    for package in index["packages"]:
        if package.get("name") != "adafruit":
            continue
        for platform in package.get("platforms", []):
            if (
                platform.get("architecture") == "nrf52"
                and platform.get("version") == ADAFRUIT_CORE_VERSION
            ):
                return package, platform
    raise RuntimeError(
        f"Adafruit nRF52 platform {ADAFRUIT_CORE_VERSION} not found in package index"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--version", default="1.0.0", help="GRASPion package version")
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

    board_fragment = script_dir / "graspionHead.boards.txt"
    variant_dir = script_dir / "variants" / "graspionHead"

    if not board_fragment.exists():
        raise FileNotFoundError(board_fragment)
    if not variant_dir.exists():
        raise FileNotFoundError(variant_dir)

    package_root_name = f"graspion-nrf52-{args.version}"
    archive_name = f"{package_root_name}.tar.bz2"
    archive_path = output_dir / archive_name
    release_tag = f"arduino-nrf52-{args.version}"
    release_url = (
        f"https://github.com/{args.repo}/releases/download/"
        f"{release_tag}/{archive_name}"
    )

    with tempfile.TemporaryDirectory() as tmp_name:
        tmp = Path(tmp_name)
        source_archive = tmp / "adafruit-nrf52.tar.gz"
        download(ADAFRUIT_CORE_URL, source_archive)

        with tarfile.open(source_archive, "r:gz") as tar:
            tar.extractall(tmp, filter="data")

        extracted = tmp / f"Adafruit_nRF52_Arduino-{ADAFRUIT_CORE_VERSION}"
        package_root = tmp / package_root_name
        shutil.copytree(extracted, package_root)

        # Expose only the GRASPion Head board in Arduino's board menu.
        board_text = board_fragment.read_text(encoding="utf-8")
        if not board_text.endswith("\n"):
            board_text += "\n"
        (package_root / "boards.txt").write_text(board_text, encoding="utf-8")

        # Remove all upstream board variants and keep only graspionHead.
        variants_root = package_root / "variants"
        if variants_root.exists():
            shutil.rmtree(variants_root)
        variants_root.mkdir(parents=True)
        shutil.copytree(variant_dir, variants_root / "graspionHead")

        with tarfile.open(archive_path, "w:bz2") as tar:
            tar.add(package_root, arcname=package_root_name)

    checksum = sha256(archive_path)
    archive_size = archive_path.stat().st_size

    with urllib.request.urlopen(ADAFRUIT_INDEX_URL) as response:
        adafruit_index = json.load(response)

    adafruit_package, adafruit_platform = find_adafruit_nrf52_platform(adafruit_index)

    # Re-use Adafruit's tool binaries and versions, but publish them under the
    # GRASPion package namespace so users only need one Board Manager URL.
    tools = adafruit_package.get("tools", [])
    tools_dependencies = []
    for dependency in adafruit_platform.get("toolsDependencies", []):
        dep = dict(dependency)
        if dep.get("packager") == "adafruit":
            dep["packager"] = "graspion"
        tools_dependencies.append(dep)

    platform = {
        "name": "GRASPion nRF52 Boards",
        "architecture": "nrf52",
        "version": args.version,
        "category": "Contributed",
        "url": release_url,
        "archiveFileName": archive_name,
        "checksum": f"SHA-256:{checksum}",
        "size": str(archive_size),
        "boards": [{"name": "GRASPion Head nRF52832"}],
        "toolsDependencies": tools_dependencies,
    }

    result = {
        "packages": [
            {
                "name": "graspion",
                "maintainer": "GRASP Lab - University of Liège",
                "websiteURL": "https://graspion.be",
                "platforms": [platform],
                "tools": tools,
            }
        ]
    }

    index_path = output_dir / "package_graspion_index.json"
    index_path.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")

    print(f"Created {archive_path}")
    print(f"SHA-256: {checksum}")
    print(f"Size: {archive_size}")
    print(f"Created {index_path}")


if __name__ == "__main__":
    main()
