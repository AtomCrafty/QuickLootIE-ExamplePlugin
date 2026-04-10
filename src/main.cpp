#include <spdlog/sinks/basic_file_sink.h>

#include "QuickLootAPI.h"

using namespace QuickLoot::API;

static void OnTakingItem(TakingItemEvent* event)
{
	logger::info("Trying to take {}", event->stack->entry->GetDisplayName());

	// Prevent enchanted items from being taken.
	// Use this mechanism to manually handle the take logic for
	// fake items you have inserted with the ModifyInventory event.
	if (event->stack->entry->IsEnchanted()) {
		event->result = HandleResult::kStop;
	}
}

static void OnTakeItem(TakeItemEvent* event)
{
	logger::info("Took {}", event->stack->entry->GetDisplayName());
}

static void OnSelectItem(SelectItemEvent* event)
{
	logger::info("Selected {}", event->stack->entry->GetDisplayName());
}

static void OnOpeningLootMenu(OpeningLootMenuEvent* event)
{
	if (const auto cont = event->container.get()) {
		logger::info("Trying to open {}", cont->GetDisplayFullName());

		// Abort the opening action if the target is a horse
		if (cont->IsHorse()) {
			event->result == HandleResult::kStop;
		}
	}
}

static void OnOpenLootMenu(OpenLootMenuEvent* event)
{
	if (const auto cont = event->container.get()) {
		logger::info("Opened {}", cont->GetDisplayFullName());
	}
}

static void OnCloseLootMenu(CloseLootMenuEvent* event)
{
	if (const auto cont = event->container.get()) {
		logger::info("Closed {}", cont->GetDisplayFullName());
	}
}

static void OnInventoryRefresh(InvalidateLootMenuEvent* event)
{
	if (const auto cont = event->container.get()) {
		logger::info("Refreshed inventory of {}", cont->GetDisplayFullName());
	}
}

static void OnModifyInventory(ModifyInventoryEvent* event)
{
	auto& inv = event->inventory;

	// Prevent ammo from showing up in the item list.
	for (int i = inv.size() - 1; i >= 0; --i) {
		if (skyrim_cast<RE::TESAmmo*>(inv[i].entry->object)) {
			logger::info("Removing {} from item list", inv[i].entry->GetDisplayName());

			// Make sure to delete the entry object!
			delete inv[i].entry;
			inv.erase(&inv[i]);
		}
	}

	// Add a fake entry of 10 gold to each container.
	// Since these items aren't real you should handle them yourself in the TakingItem event and prevent the default action.
	static auto gold = RE::TESForm::LookupByID<RE::TESObjectMISC>(0xf);

	// Simply allocate the InventoryEntryData with the new operator.
	// QuickLoot will delete it when it is no longer needed.
	inv.push_back({ new RE::InventoryEntryData{gold, 10} });
}

static void OnPopulateInfoBar(PopulateInfoBarEvent* event)
{
	if (!event->stack) return;

	logger::info("Populating info bar for {}", event->stack->entry->GetDisplayName());

	// Provide damage and armor values in the info bar.
	if (skyrim_cast<RE::TESObjectARMO*>(event->stack->entry->object)) {
		event->result.push_back(std::format("Armor: {:2g}", RE::PlayerCharacter::GetSingleton()->GetArmorValue(event->stack->entry)).c_str());
	}
	else if (skyrim_cast<RE::TESObjectWEAP*>(event->stack->entry->object)) {
		event->result.push_back(std::format("Damage: {:2g}", RE::PlayerCharacter::GetSingleton()->GetDamage(event->stack->entry)).c_str());
	}
}

static void OnPopulateButtonBar(PopulateButtonBarEvent* event)
{
	if (!event->stack) return;

	logger::info("Populating button bar for {}", event->stack->entry->GetDisplayName());

	event->result.push_back({ "Useless Button", 28 });
}

void OnSKSEMessage(SKSE::MessagingInterface::Message* msg)
{
	switch (msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		QuickLootAPI::Init("QuickLoot IE Example Plugin");

		QuickLootAPI::RegisterTakingItemHandler(OnTakingItem);
		QuickLootAPI::RegisterTakeItemHandler(OnTakeItem);
		QuickLootAPI::RegisterSelectItemHandler(OnSelectItem);

		QuickLootAPI::RegisterOpeningLootMenuHandler(OnOpeningLootMenu);
		QuickLootAPI::RegisterOpenLootMenuHandler(OnOpenLootMenu);
		QuickLootAPI::RegisterCloseLootMenuHandler(OnCloseLootMenu);

		QuickLootAPI::RegisterInvalidateLootMenuHandler(OnInventoryRefresh);
		QuickLootAPI::RegisterModifyInventoryHandler(OnModifyInventory);

		QuickLootAPI::RegisterPopulateInfoBarHandler(OnPopulateInfoBar);
		QuickLootAPI::RegisterPopulateButtonBarHandler(OnPopulateButtonBar);
		break;

	default:
		break;
	}
}

static void InitializeLog(spdlog::level::level_enum level = spdlog::level::info)
{
	auto path = logger::log_directory();
	if (!path) {
		SKSE::stl::report_and_fail("Failed to find standard logging directory");
	}

	const auto plugin = SKSE::PluginDeclaration::GetSingleton();

	*path /= std::format("{}.log", plugin->GetName());
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
	auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S.%e] [%l] [%t] %v");
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	SKSE::Init(a_skse, false);
	return SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessage);
}
