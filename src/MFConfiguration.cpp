/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Neil Enns. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <FlashIAP.h>
#include <fmt/core.h>
#include <map>
#include <mbed.h>
#include <TextLCD.h>

#include "Globals.hpp"
#include "MFCommands.hpp"
#include "MFConfiguration.hpp"

#define FLASH_USER_DATA_START 0x080E0000
#define FLASH_USER_DATA_SIZE 2048

inline constexpr auto flashStorageVersion = "1.0.0"sv;

static char userConfig[FLASH_USER_DATA_SIZE] __attribute__((__section__(".user_data")));

void MFConfiguration::AddButton(ARDUINO_PIN arduinoPinName, char const *name)
{
  if (pinManager.IsPinRegistered(arduinoPinName))
  {
#ifdef DEBUG
    cmdMessenger.sendCmd(MFCommand::kStatus, "Duplicate pin.");
#endif
    return;
  }

  buttons.insert({arduinoPinName, std::make_shared<MFButton>(arduinoPinName, name)});
  pinManager.RegisterPin(arduinoPinName);
}

void MFConfiguration::AddLcdDisplay(int address, int rows, int columns, char const *name)
{
  if (lcdDisplays.count(address))
  {
#ifdef DEBUG
    cmdMessenger.sendCmd(MFCommand::kStatus, "Duplicate LCD display address.");
#endif
    return;
  }

  lcdDisplays.insert({address, std::make_shared<MFLcdDisplay>(address, rows, columns, name)});
}

void MFConfiguration::AddLedDisplay(ARDUINO_PIN mosi, ARDUINO_PIN sclk, ARDUINO_PIN cs, int submoduleCount, char const *name)
{
  if (pinManager.IsPinRegistered(mosi) || pinManager.IsPinRegistered(sclk) || pinManager.IsPinRegistered(cs))
  {
#ifdef DEBUG
    cmdMessenger.sendCmd(MFCommand::kStatus, "Duplicate pin.");
#endif
    return;
  }

  // MOSI a.k.a. the data pin is used as the identifier for the connected module
  // by MobiFlight.
  ledDisplays.insert({mosi, std::make_shared<MFMAX7219>(mosi, sclk, cs, submoduleCount, name)});
  pinManager.RegisterPin(mosi);
  pinManager.RegisterPin(sclk);
  pinManager.RegisterPin(cs);
}

void MFConfiguration::AddOutput(ARDUINO_PIN arduinoPinName, char const *name)
{
  if (pinManager.IsPinRegistered(arduinoPinName))
  {
#ifdef DEBUG
    cmdMessenger.sendCmd(MFCommand::kStatus, "Duplicate pin.");
#endif
    return;
  }

  outputs.insert({arduinoPinName, std::make_shared<MFOutput>(arduinoPinName, name)});
  pinManager.RegisterPin(arduinoPinName);
}

void MFConfiguration::AddServo(ARDUINO_PIN arduinoPinName, char const *name)
{
  if (pinManager.IsPinRegistered(arduinoPinName))
  {
#ifdef DEBUG
    cmdMessenger.sendCmd(MFCommand::kStatus, "Duplicate pin.");
#endif
    return;
  }

  servos.insert({arduinoPinName, std::make_shared<MFServo>(arduinoPinName, name)});
  pinManager.RegisterPin(arduinoPinName);
}

void MFConfiguration::Load()
{
  // The first test of a valid configuration is to check for MF; stored
  // at the appropriate location in flash. If it's not there
  if (strncmp(userConfig, flashIdentifier.data(), 2) != 0)
  {
    cmdMessenger.sendCmd(MFCommand::kStatus, "No configuration saved in flash.");
    return;
  }

  const std::string_view storedConfiguration(userConfig);
  cmdMessenger.sendCmd(MFCommand::kGetConfig, userConfig);
}

void MFConfiguration::Erase()
{
}

void MFConfiguration::Save()
{
  auto flash = new FlashIAP();
  std::string buffer;

  buffer.append(fmt::format("{}:{}:", flashIdentifier, flashStorageVersion));
  Serialize(buffer);

  flash->init();
  volatile auto sectorSize = flash->get_sector_size(FLASH_USER_DATA_START);
  volatile auto padAmount = sectorSize - (sectorSize % buffer.length());

  // Pad the string out to a multiple of the sector size
  buffer.append(padAmount, '\0');
  auto status = flash->erase(FLASH_USER_DATA_START, FLASH_USER_DATA_SIZE);
  flash->program(buffer.c_str(), FLASH_USER_DATA_START, buffer.length());
  flash->deinit();

  cmdMessenger.sendCmd(kStatus, "Configuration saved");
}

void MFConfiguration::Serialize(std::string &buffer)
{
  for (auto &[key, output] : outputs)
  {
    output->Serialize(buffer);
  }

  for (auto &[key, button] : buttons)
  {
    button->Serialize(buffer);
  }

  for (auto &[key, ledDisplay] : ledDisplays)
  {
    ledDisplay->Serialize(buffer);
  }

  for (auto &[key, lcdDisplay] : lcdDisplays)
  {
    lcdDisplay->Serialize(buffer);
  }

  for (auto &[key, servo] : servos)
  {
    servo->Serialize(buffer);
  }
}

void MFConfiguration::StartTest()
{
  for (auto &[key, value] : servos)
  {
    value->StartTest();
  }

  // for (auto &[key, value] : ledDisplays)
  // {
  //   value->StartTest();
  // }

  // for (auto &[key, value] : lcdDisplays)
  // {
  //   value->StartTest();
  // }
  // for (auto &[key, value] : outputs)
  // {
  //   value->StartTest();
  // }
}

void MFConfiguration::StopTest()
{
  for (auto &[key, value] : lcdDisplays)
  {
    value->StopTest();
  }

  for (auto &[key, value] : ledDisplays)
  {
    value->StopTest();
  }

  for (auto &[key, value] : outputs)
  {
    value->StopTest();
  }
}