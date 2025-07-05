/*
 *  Copyright (c) 2013 Colin Svingen
 *	Copyright (c) 2025 Hafidh Hidayat
 *
 *  Gaming KB USB Keyboard support
 *  Original work: https://github.com/Swoogan/aziokbd
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

/*
 * Version Information
 */
#define DRIVER_VERSION ""
#define DRIVER_AUTHOR "Hafidh Hidayat <hafidhhidayat@hotmail.com>"
#define DRIVER_DESC "GAMING KB Keyboard Driver"
#define DRIVER_LICENSE "GPL"
#define ML_VENDOR_ID   0x258a
#define ML_PRODUCT_ID  0x1006

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

static const unsigned char usb_kbd_keycode[256] = {
		/* BEGIN 06 */
/* 0-7 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 8-15 */	KEY_LEFTCTRL, KEY_LEFTSHIFT, KEY_LEFTALT, KEY_LEFTMETA, KEY_RIGHTCTRL, KEY_RIGHTSHIFT, KEY_RIGHTALT, KEY_RESERVED,
/* 16-23 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_A, KEY_B, KEY_C, KEY_D,
/* 24-31 */	KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L,
/* 32-39 */	KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
/* 40-47 */	KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_1, KEY_2,
/* 48-55 */	KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
/* 56-63 */	KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB, KEY_SPACE, KEY_MINUS, KEY_EQUAL, KEY_LEFTBRACE,
		/* END 06 */

		/* BEGIN 07 */
/* 64-71 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 72-79 */	KEY_RIGHTBRACE, KEY_BACKSLASH, KEY_RESERVED, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE, KEY_COMMA, KEY_DOT,
/* 80-87 */	KEY_SLASH, KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
/* 88-95 */	KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_SYSRQ, KEY_SCROLLLOCK,
/* 96-103 */	KEY_INSERT, KEY_PAUSE, KEY_HOME, KEY_PAGEUP, KEY_DELETE, KEY_END, KEY_PAGEDOWN, KEY_RIGHT,
/* 104-111 */   KEY_LEFT, KEY_DOWN, KEY_UP, KEY_NUMLOCK, KEY_KPSLASH, KEY_KPASTERISK, KEY_KPMINUS, KEY_KPPLUS,
/* 112-119 */   KEY_KPENTER, KEY_KP1, KEY_KP2, KEY_KP3, KEY_KP4, KEY_KP5, KEY_KP6, KEY_KP7,
/* 120-127 */   KEY_KP8, KEY_KP9, KEY_KP0, KEY_KPDOT, KEY_102ND, KEY_MENU, KEY_RESERVED, KEY_RESERVED,
		/* END 07 */

		/* BEGIN 02 */
/* 128-135 */   KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_MEDIA, KEY_MUTE, KEY_STOPCD, KEY_PREVIOUSSONG, KEY_PLAYPAUSE, KEY_NEXTSONG,
/* 136-143 */   KEY_MAIL, KEY_HOMEPAGE, KEY_CALC, KEY_CYCLEWINDOWS, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 144-151 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 152-159 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 160-167 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 168-175 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 176-183 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 184-191 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
		/* END 02 */

		/* Not used */
/* 192-199 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 200-207 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 208-215 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 216-223 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 224-231 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_SLASH,
/* 232-239 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 240-247 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 248-255 */   KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
		/* Not used */
};

/**
 * struct usb_kbd - state of each attached keyboard
 *
 * @dev:        input device associated with this keyboard
 * @usbdev:     usb device associated with this keyboard
 * @old:        data received in the past from the @irq URB representing which
 *              keys were pressed. By comparing with the current list of keys
 *              that are pressed, we are able to see key releases.
 * @irq:        URB for receiving a list of keys that are pressed when a
 *              new key is pressed or a key that was pressed is released.
 * @led:        URB for sending LEDs (e.g. numlock, ...)
 * @newleds:    data that will be sent with the @led URB representing which LEDs
 *              should be on
 * @name:       Name of the keyboard. @dev's name field points to this buffer
 * @phys:       Physical path of the keyboard. @dev's phys field points to this
 *              buffer
 * @new:        Buffer for the @irq URB
 * @cr:         Control request for @led URB
 * @leds:       Buffer for the @led URB
 * @new_dma:    DMA address for @irq URB
 * @leds_dma:   DMA address for @led URB
 * @leds_lock:  spinlock that protects @leds, @newleds, and @led_urb_submitted
 * @led_urb_submitted: indicates whether @led is in progress, i.e. it has been
 *              submitted and its completion handler has not returned yet
 *              without resubmitting @led
 */
struct usb_kbd {
	struct input_dev *dev;
	struct usb_device *usbdev;
	unsigned char old_01[8];
	unsigned char old_04[8];
	unsigned char old_05[8];
	unsigned char old_06[8];
	struct urb *irq, *led;
	unsigned char newleds;
	char name[128];
	char phys[64];

	unsigned char *new;
	struct usb_ctrlrequest *cr;
	unsigned char *leds;
	dma_addr_t new_dma;
	dma_addr_t leds_dma;
};

static void usb_kbd_irq(struct urb *urb)
{
	struct usb_kbd *kbd = urb->context;
	int i, j, offset;
	unsigned char modified_mask;

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	/* -EPIPE:  should clear the halt */
	default:		/* error */
		goto resubmit;
	}

	if (kbd->new[0] == 2) {
		/* volume down */
		if (kbd->new[1] == 234 && kbd->old_01[1] != 234)
			input_report_key(kbd->dev, usb_kbd_keycode[128], 1);
		if (kbd->old_01[1] == 234 && kbd->new[1] != 234)
			input_report_key(kbd->dev, usb_kbd_keycode[128], 0);

		/* volume up */
		if (kbd->new[1] == 233 && kbd->old_01[1] != 233)
			input_report_key(kbd->dev, usb_kbd_keycode[129], 1);
		if (kbd->old_01[1] == 233 && kbd->new[1] != 233)
			input_report_key(kbd->dev, usb_kbd_keycode[129], 0);

		/* Media */
		if (kbd->new[1] == 131 && kbd->old_01[1] != 131)
			input_report_key(kbd->dev, usb_kbd_keycode[130], 1);
		if (kbd->old_01[1] == 131 && kbd->new[1] != 131)
			input_report_key(kbd->dev, usb_kbd_keycode[130], 0);

		/* Mute */
		if (kbd->new[1] == 226 && kbd->old_01[1] != 226)
			input_report_key(kbd->dev, usb_kbd_keycode[131], 1);
		if (kbd->old_01[1] == 226 && kbd->new[1] != 226)
			input_report_key(kbd->dev, usb_kbd_keycode[131], 0);

		/* Stop */
		if (kbd->new[1] == 183 && kbd->old_01[1] != 183)
			input_report_key(kbd->dev, usb_kbd_keycode[132], 1);
		if (kbd->old_01[1] == 183 && kbd->new[1] != 183)
			input_report_key(kbd->dev, usb_kbd_keycode[132], 0);

		/* Prev Song */
		if (kbd->new[1] == 182 && kbd->old_01[1] != 182)
			input_report_key(kbd->dev, usb_kbd_keycode[133], 1);
		if (kbd->old_01[1] == 182 && kbd->new[1] != 182)
			input_report_key(kbd->dev, usb_kbd_keycode[133], 0);

		/* Play/Pause */
		if (kbd->new[1] == 205 && kbd->old_01[1] != 205)
			input_report_key(kbd->dev, usb_kbd_keycode[134], 1);
		if (kbd->old_01[1] == 205 && kbd->new[1] != 205)
			input_report_key(kbd->dev, usb_kbd_keycode[134], 0);

		/* Next Song */
		if (kbd->new[1] == 181 && kbd->old_01[1] != 181)
			input_report_key(kbd->dev, usb_kbd_keycode[135], 1);
		if (kbd->old_01[1] == 181 && kbd->new[1] != 181)
			input_report_key(kbd->dev, usb_kbd_keycode[135], 0);

		/* Mail */
		if (kbd->new[1] == 138 && kbd->old_01[1] != 138)
			input_report_key(kbd->dev, usb_kbd_keycode[136], 1);
		if (kbd->old_01[1] == 138 && kbd->new[1] != 138)
			input_report_key(kbd->dev, usb_kbd_keycode[136], 0);

		/* Homepage */
		if (kbd->new[1] == 35 && kbd->old_01[1] != 35)
			input_report_key(kbd->dev, usb_kbd_keycode[137], 1);
		if (kbd->old_01[1] == 35 && kbd->new[1] != 35)
			input_report_key(kbd->dev, usb_kbd_keycode[137], 0);

		/* Calc */
		if (kbd->new[1] == 146 && kbd->old_01[1] != 146)
			input_report_key(kbd->dev, usb_kbd_keycode[138], 1);
		if (kbd->old_01[1] == 146 && kbd->new[1] != 146)
			input_report_key(kbd->dev, usb_kbd_keycode[138], 0);

		/* Computer */
		if (kbd->new[1] == 148 && kbd->old_01[1] != 148)
			input_report_key(kbd->dev, usb_kbd_keycode[139], 1);
		if (kbd->old_01[1] == 148 && kbd->new[1] != 148)
			input_report_key(kbd->dev, usb_kbd_keycode[139], 0);
		memcpy(kbd->old_01, kbd->new, 8);
	} else if (kbd->new[0] == 6) {
		for (j = 1; j < 8; j++) {
			offset = (j * 8);
			modified_mask = kbd->new[j] ^ kbd->old_06[j];
			kbd->old_06[j] = kbd->new[j];
			for (i = 0; i < 8; i++) {
				if (modified_mask & 1) {
					input_report_key(kbd->dev,
							 usb_kbd_keycode[offset + i],
							 (kbd->new[j] & 1));
				}
				modified_mask >>= 1;
				kbd->new[j] >>= 1;
			}
		}
	} else if (kbd->new[0] == 7) {
		for (j = 1; j < 8; j++) {
			offset = (j * 8) + 64;
			modified_mask = kbd->new[j] ^ kbd->old_05[j];
			kbd->old_05[j] = kbd->new[j];
			for (i = 0; i < 8; i++) {
				if (modified_mask & 1) {
					input_report_key(kbd->dev,
							 usb_kbd_keycode[offset + i],
							 (kbd->new[j] & 1));
				}
				modified_mask >>= 1;
				kbd->new[j] >>= 1;
			}
		}
	}

	input_sync(kbd->dev);


resubmit:
	i = usb_submit_urb(urb, GFP_ATOMIC);
	if (i)
		hid_err(urb->dev,
			"can't resubmit intr, %s-%s/input0, status %d",
			kbd->usbdev->bus->bus_name,
			kbd->usbdev->devpath, i);
}

static int usb_kbd_event(struct input_dev *dev, unsigned int type,
			 unsigned int code, int value)
{
	struct usb_kbd *kbd = input_get_drvdata(dev);

	if (type != EV_LED)
		return -1;

	kbd->newleds = (!!test_bit(LED_KANA,    dev->led) << 4) |
		       (!!test_bit(LED_COMPOSE, dev->led) << 3) |
		       (!!test_bit(LED_SCROLLL, dev->led) << 2) |
		       (!!test_bit(LED_CAPSL,   dev->led) << 1) |
		       (!!test_bit(LED_NUML,    dev->led));

	if (kbd->led->status == -EINPROGRESS)
		return 0;

	if (*(kbd->leds) == kbd->newleds)
		return 0;

	*(kbd->leds) = kbd->newleds;
	kbd->led->dev = kbd->usbdev;
	if (usb_submit_urb(kbd->led, GFP_ATOMIC))
		pr_err("usb_submit_urb(leds) failed\n");

	return 0;
}

static void usb_kbd_led(struct urb *urb)
{
	struct usb_kbd *kbd = urb->context;

	if (urb->status)
		hid_warn(urb->dev, "led urb status %d received\n",
			 urb->status);

	if (*(kbd->leds) == kbd->newleds)
		return;

	*(kbd->leds) = kbd->newleds;
	kbd->led->dev = kbd->usbdev;

	if (usb_submit_urb(kbd->led, GFP_ATOMIC))
		hid_err(urb->dev, "usb_submit_urb(leds) failed\n");
}

static int usb_kbd_open(struct input_dev *dev)
{
	struct usb_kbd *kbd = input_get_drvdata(dev);

	kbd->irq->dev = kbd->usbdev;
	if (usb_submit_urb(kbd->irq, GFP_KERNEL))
		return -EIO;

	return 0;
}

static void usb_kbd_close(struct input_dev *dev)
{
	struct usb_kbd *kbd = input_get_drvdata(dev);

	usb_kill_urb(kbd->irq);
}

static int usb_kbd_alloc_mem(struct usb_device *dev, struct usb_kbd *kbd)
{
	kbd->irq = usb_alloc_urb(0, GFP_KERNEL);
	if (!kbd->irq)
		return -1;
	kbd->led = usb_alloc_urb(0, GFP_KERNEL);
	if (!kbd->led)
		return -1;
	kbd->new = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &kbd->new_dma);
	if (!kbd->new)
		return -1;
	kbd->cr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL);
	if (!kbd->cr)
		return -1;
	kbd->leds = usb_alloc_coherent(dev, 1, GFP_ATOMIC, &kbd->leds_dma);
	if (!kbd->leds)
		return -1;

	return 0;
}

static void usb_kbd_free_mem(struct usb_device *dev, struct usb_kbd *kbd)
{
	usb_free_urb(kbd->irq);
	usb_free_urb(kbd->led);
	usb_free_coherent(dev, 8, kbd->new, kbd->new_dma);
	kfree(kbd->cr);
	usb_free_coherent(dev, 1, kbd->leds, kbd->leds_dma);
}

static int usb_kbd_probe(struct usb_interface *iface,
			 const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(iface);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_kbd *kbd;
	struct input_dev *input_dev;
	int i, pipe, maxp;
	int error = -ENOMEM;

	interface = iface->cur_altsetting;

	if (interface->desc.bNumEndpoints != 1)
		return -ENODEV;

	endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint))
		return -ENODEV;

	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	maxp = usb_maxpacket(dev, pipe);
	kbd = kzalloc(sizeof(struct usb_kbd), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!kbd || !input_dev)
		goto fail1;

	if (usb_kbd_alloc_mem(dev, kbd))
		goto fail2;

	kbd->usbdev = dev;
	kbd->dev = input_dev;

	if (dev->manufacturer)
		strncpy(kbd->name, dev->manufacturer, sizeof(kbd->name));

	if (dev->product) {
		if (dev->manufacturer)
			strlcat(kbd->name, " ", sizeof(kbd->name));
		strlcat(kbd->name, dev->product, sizeof(kbd->name));
	}

	if (!strlen(kbd->name))
		snprintf(kbd->name, sizeof(kbd->name),
			 "USB HIDBP Keyboard %04x:%04x",
			 le16_to_cpu(dev->descriptor.idVendor),
			 le16_to_cpu(dev->descriptor.idProduct));

	printk("<1>GAMING KB: detected %s\n", kbd->name);

	usb_make_path(dev, kbd->phys, sizeof(kbd->phys));
	strlcat(kbd->phys, "/input0", sizeof(kbd->phys));

	input_dev->name = kbd->name;
	input_dev->phys = kbd->phys;
	usb_to_input_id(dev, &input_dev->id);
	input_dev->dev.parent = &iface->dev;

	input_set_drvdata(input_dev, kbd);

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_LED) |
		BIT_MASK(EV_REP);
	input_dev->ledbit[0] = BIT_MASK(LED_NUML) | BIT_MASK(LED_CAPSL) |
		BIT_MASK(LED_SCROLLL) | BIT_MASK(LED_COMPOSE) |
		BIT_MASK(LED_KANA);

	for (i = 0; i < 255; i++)
		set_bit(usb_kbd_keycode[i], input_dev->keybit);

	clear_bit(0, input_dev->keybit);

	input_dev->event = usb_kbd_event;
	input_dev->open = usb_kbd_open;
	input_dev->close = usb_kbd_close;

	usb_fill_int_urb(kbd->irq, dev, pipe,
			 kbd->new, (maxp > 8 ? 8 : maxp),
			 usb_kbd_irq, kbd, endpoint->bInterval);
	kbd->irq->transfer_dma = kbd->new_dma;
	kbd->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	kbd->cr->bRequestType = USB_TYPE_CLASS | USB_RECIP_INTERFACE;
	kbd->cr->bRequest = 0x09;
	kbd->cr->wValue = cpu_to_le16(0x200);
	kbd->cr->wIndex = cpu_to_le16(0);
	kbd->cr->wLength = cpu_to_le16(1);

	usb_fill_control_urb(kbd->led, dev, usb_sndctrlpipe(dev, 0),
			     (void *) kbd->cr, kbd->leds, 1,
			     usb_kbd_led, kbd);
	kbd->led->transfer_dma = kbd->leds_dma;
	kbd->led->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	error = input_register_device(kbd->dev);
	if (error)
		goto fail2;

	usb_set_intfdata(iface, kbd);
	device_set_wakeup_enable(&dev->dev, 1);
	return 0;

fail2:
	usb_kbd_free_mem(dev, kbd);
fail1:
	input_free_device(input_dev);
	kfree(kbd);
	return error;
}

static void usb_kbd_disconnect(struct usb_interface *intf)
{
	struct usb_kbd *kbd = usb_get_intfdata(intf);

	usb_set_intfdata(intf, NULL);
	if (kbd) {
		usb_kill_urb(kbd->irq);
		input_unregister_device(kbd->dev);
		usb_kbd_free_mem(interface_to_usbdev(intf), kbd);
		kfree(kbd);
	}
}

static struct usb_device_id usb_kbd_id_table[] = {
	{ USB_DEVICE(ML_VENDOR_ID, ML_PRODUCT_ID) },
	{ }						/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, usb_kbd_id_table);

static struct usb_driver usb_kbd_driver = {
	.name =		"GAMING KB",
	.probe =	usb_kbd_probe,
	.disconnect =	usb_kbd_disconnect,
	.id_table =	usb_kbd_id_table,
};

static int __init usb_kbd_init(void)
{
	int result = usb_register(&usb_kbd_driver);

	if (result == 0)
		printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
				DRIVER_DESC "\n");
	return result;
}

static void __exit usb_kbd_exit(void)
{
	usb_deregister(&usb_kbd_driver);
}

module_init(usb_kbd_init);
module_exit(usb_kbd_exit);