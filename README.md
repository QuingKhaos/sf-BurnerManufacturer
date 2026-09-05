[![Discord community](https://img.shields.io/badge/Discord-%235865F2.svg?style=for-the-badge&logo=discord&logoColor=white)](https://discord.gg/B3qJBE3M9W) [![Satisfactory mod portal](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fapi.ficsit.app%2Fv1%2Fmod%2FMODID&query=data.downloads&suffix=%20downloads&style=for-the-badge&color=orange&label=ficsit.app&labelColor=rgb(111,148,173))](https://ficsit.app/mod/BurnerManufacturer) ![Multiplayer supported](https://img.shields.io/badge/Multiplayer-Supported-green?style=for-the-badge) [![GitHub issues: bugs](https://img.shields.io/github/issues/QuingKhaos/sf-BurnerManufacturer/bug?label=Bug%20Reports&style=for-the-badge&logo=github)](https://github.com/QuingKhaos/sf-BurnerManufacturer/issues?q=is%3Aissue%20state%3Aopen%20label%3Abug) [![GitHub pull requests](https://img.shields.io/github/issues-pr/QuingKhaos/sf-BurnerManufacturer?label=Pull%20Requests&style=for-the-badge&logo=github)](https://github.com/QuingKhaos/sf-BurnerManufacturer/pulls)

# Burner Production Machines

Library mod that provides a buildable manufacturer base class that runs on solid fuel instead of electricity. This mod does nothing on its own, but it is a dependency for other mods that make use of production machines that run on solid fuel.

## For modders

Create your production machine as a subclass `KhaosBuildableManufacturerBurner` instead of `FGBuildableManufacturer`. Set the power usage and default allowed fuel classes in the class properties. The burner manufacturer is looking for a `FGFactoryConnectionComponent` name prefixed with `FuelInput` as fuel input connection to pull the solid fuels from the conveyor belt.

## License

This mod is licensed under the [GNU GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html). You are free to use, modify, and distribute this mod under the same terms of the license. If you make use of the Burner Production Machines in your own mod, your mod must be open-source, the source linked on SMR, and your mod also be licensed under the GNU GPLv3.
