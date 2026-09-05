#pragma once

#include "Logging/LogMacros.h"

BURNERMANUFACTURER_API DECLARE_LOG_CATEGORY_EXTERN(LogBurnerManufacturer, Verbose, All);

/**
 * Creates appropriate messages for the LogBurnerManufacturer category.
 *
 * @param Verbosity Verbosity level of this message. See ELogVerbosity.
 * @param Message Message string literal.
 */
#define KBM_LOG(Verbosity, Message) \
	UE_LOG(LogBurnerManufacturer, Verbosity, TEXT("[%s]: %s"), *FString::Printf(TEXT("%s::%s"), *GetClass()->GetName(), ANSI_TO_TCHAR(__FUNCTION__)), Message);

 /**
  * Creates appropriate messages for the LogBurnerManufacturer category, with format arguments.
  *
  * @param Verbosity Verbosity level of this message. See ELogVerbosity.
  * @param Format Format string literal in the style of printf.
  * @param Args Comma-separated arguments used to format the message.
  */
#define KBM_LOG_ARGS(Verbosity, Format, ...) \
	UE_LOG(LogBurnerManufacturer, Verbosity, TEXT("[%s]: %s"), *FString::Printf(TEXT("%s::%s"), *GetClass()->GetName(), ANSI_TO_TCHAR(__FUNCTION__)), *FString::Printf(Format, ##__VA_ARGS__));
