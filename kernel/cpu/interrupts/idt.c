#include <stdint.h>

#include <io/printf.h>
#include "idt.h"

static struct idtr idt_reg;
static struct idt_entry idt[256];

static void
idt_register_entry(uint8_t vector, void* handler, uint8_t flags)
{
    idt[vector] = (struct idt_entry){
        .offset_low  = (uint64_t)handler & 0xFFFF,
        .offset_mid  = ((uint64_t)handler >> 16) & 0xFFFF,
        .offset_high = ((uint64_t)handler >> 32) & 0xFFFFFFFF,

        .flags = flags,
        .segment_selector = KERNEL_MODE_CODE_SEGMENT,

        .ist      = 0,
        .reserved = 0,
    };
}

static void
idt_reload(void)
{
    idt_reg = (struct idtr) {
        .base  = (uint64_t)idt,
        .limit = sizeof(idt) - 1
    };

    asm volatile ("lidt %0" :: "m"(idt_reg));
}

static void
idt_register_entries(void)
{
    idt_register_entry(0, isr0, INT_GATE);
    idt_register_entry(1, isr1, INT_GATE);
    idt_register_entry(2, isr2, INT_GATE);
    idt_register_entry(3, isr3, INT_GATE);
    idt_register_entry(4, isr4, INT_GATE);
    idt_register_entry(5, isr5, INT_GATE);
    idt_register_entry(6, isr6, INT_GATE);
    idt_register_entry(7, isr7, INT_GATE);
    idt_register_entry(8, isr8, INT_GATE);
    idt_register_entry(9, isr9, INT_GATE);
    idt_register_entry(10, isr10, INT_GATE);
    idt_register_entry(11, isr11, INT_GATE);
    idt_register_entry(12, isr12, INT_GATE);
    idt_register_entry(13, isr13, INT_GATE);
    idt_register_entry(14, isr14, INT_GATE);
    idt_register_entry(15, isr15, INT_GATE);
    idt_register_entry(16, isr16, INT_GATE);
    idt_register_entry(17, isr17, INT_GATE);
    idt_register_entry(18, isr18, INT_GATE);
    idt_register_entry(19, isr19, INT_GATE);
    idt_register_entry(20, isr20, INT_GATE);
    idt_register_entry(21, isr21, INT_GATE);
    idt_register_entry(22, isr22, INT_GATE);
    idt_register_entry(23, isr23, INT_GATE);
    idt_register_entry(24, isr24, INT_GATE);
    idt_register_entry(25, isr25, INT_GATE);
    idt_register_entry(26, isr26, INT_GATE);
    idt_register_entry(27, isr27, INT_GATE);
    idt_register_entry(28, isr28, INT_GATE);
    idt_register_entry(29, isr29, INT_GATE);
    idt_register_entry(30, isr30, INT_GATE);
    idt_register_entry(31, isr31, INT_GATE);
    idt_register_entry(32, isr32, INT_GATE);
    idt_register_entry(33, isr33, INT_GATE);
    idt_register_entry(34, isr34, INT_GATE);
    idt_register_entry(35, isr35, INT_GATE);
    idt_register_entry(36, isr36, INT_GATE);
    idt_register_entry(37, isr37, INT_GATE);
    idt_register_entry(38, isr38, INT_GATE);
    idt_register_entry(39, isr39, INT_GATE);
    idt_register_entry(40, isr40, INT_GATE);
    idt_register_entry(41, isr41, INT_GATE);
    idt_register_entry(42, isr42, INT_GATE);
    idt_register_entry(43, isr43, INT_GATE);
    idt_register_entry(44, isr44, INT_GATE);
    idt_register_entry(45, isr45, INT_GATE);
    idt_register_entry(46, isr46, INT_GATE);
    idt_register_entry(47, isr47, INT_GATE);
    idt_register_entry(48, isr48, INT_GATE);
    idt_register_entry(49, isr49, INT_GATE);
    idt_register_entry(50, isr50, INT_GATE);
    idt_register_entry(51, isr51, INT_GATE);
    idt_register_entry(52, isr52, INT_GATE);
    idt_register_entry(53, isr53, INT_GATE);
    idt_register_entry(54, isr54, INT_GATE);
    idt_register_entry(55, isr55, INT_GATE);
    idt_register_entry(56, isr56, INT_GATE);
    idt_register_entry(57, isr57, INT_GATE);
    idt_register_entry(58, isr58, INT_GATE);
    idt_register_entry(59, isr59, INT_GATE);
    idt_register_entry(60, isr60, INT_GATE);
    idt_register_entry(61, isr61, INT_GATE);
    idt_register_entry(62, isr62, INT_GATE);
    idt_register_entry(63, isr63, INT_GATE);
    idt_register_entry(64, isr64, INT_GATE);
    idt_register_entry(65, isr65, INT_GATE);
    idt_register_entry(66, isr66, INT_GATE);
    idt_register_entry(67, isr67, INT_GATE);
    idt_register_entry(68, isr68, INT_GATE);
    idt_register_entry(69, isr69, INT_GATE);
    idt_register_entry(70, isr70, INT_GATE);
    idt_register_entry(71, isr71, INT_GATE);
    idt_register_entry(72, isr72, INT_GATE);
    idt_register_entry(73, isr73, INT_GATE);
    idt_register_entry(74, isr74, INT_GATE);
    idt_register_entry(75, isr75, INT_GATE);
    idt_register_entry(76, isr76, INT_GATE);
    idt_register_entry(77, isr77, INT_GATE);
    idt_register_entry(78, isr78, INT_GATE);
    idt_register_entry(79, isr79, INT_GATE);
    idt_register_entry(80, isr80, INT_GATE);
    idt_register_entry(81, isr81, INT_GATE);
    idt_register_entry(82, isr82, INT_GATE);
    idt_register_entry(83, isr83, INT_GATE);
    idt_register_entry(84, isr84, INT_GATE);
    idt_register_entry(85, isr85, INT_GATE);
    idt_register_entry(86, isr86, INT_GATE);
    idt_register_entry(87, isr87, INT_GATE);
    idt_register_entry(88, isr88, INT_GATE);
    idt_register_entry(89, isr89, INT_GATE);
    idt_register_entry(90, isr90, INT_GATE);
    idt_register_entry(91, isr91, INT_GATE);
    idt_register_entry(92, isr92, INT_GATE);
    idt_register_entry(93, isr93, INT_GATE);
    idt_register_entry(94, isr94, INT_GATE);
    idt_register_entry(95, isr95, INT_GATE);
    idt_register_entry(96, isr96, INT_GATE);
    idt_register_entry(97, isr97, INT_GATE);
    idt_register_entry(98, isr98, INT_GATE);
    idt_register_entry(99, isr99, INT_GATE);
    idt_register_entry(100, isr100, INT_GATE);
    idt_register_entry(101, isr101, INT_GATE);
    idt_register_entry(102, isr102, INT_GATE);
    idt_register_entry(103, isr103, INT_GATE);
    idt_register_entry(104, isr104, INT_GATE);
    idt_register_entry(105, isr105, INT_GATE);
    idt_register_entry(106, isr106, INT_GATE);
    idt_register_entry(107, isr107, INT_GATE);
    idt_register_entry(108, isr108, INT_GATE);
    idt_register_entry(109, isr109, INT_GATE);
    idt_register_entry(110, isr110, INT_GATE);
    idt_register_entry(111, isr111, INT_GATE);
    idt_register_entry(112, isr112, INT_GATE);
    idt_register_entry(113, isr113, INT_GATE);
    idt_register_entry(114, isr114, INT_GATE);
    idt_register_entry(115, isr115, INT_GATE);
    idt_register_entry(116, isr116, INT_GATE);
    idt_register_entry(117, isr117, INT_GATE);
    idt_register_entry(118, isr118, INT_GATE);
    idt_register_entry(119, isr119, INT_GATE);
    idt_register_entry(120, isr120, INT_GATE);
    idt_register_entry(121, isr121, INT_GATE);
    idt_register_entry(122, isr122, INT_GATE);
    idt_register_entry(123, isr123, INT_GATE);
    idt_register_entry(124, isr124, INT_GATE);
    idt_register_entry(125, isr125, INT_GATE);
    idt_register_entry(126, isr126, INT_GATE);
    idt_register_entry(127, isr127, INT_GATE);
    idt_register_entry(128, isr128, INT_GATE);
    idt_register_entry(129, isr129, INT_GATE);
    idt_register_entry(130, isr130, INT_GATE);
    idt_register_entry(131, isr131, INT_GATE);
    idt_register_entry(132, isr132, INT_GATE);
    idt_register_entry(133, isr133, INT_GATE);
    idt_register_entry(134, isr134, INT_GATE);
    idt_register_entry(135, isr135, INT_GATE);
    idt_register_entry(136, isr136, INT_GATE);
    idt_register_entry(137, isr137, INT_GATE);
    idt_register_entry(138, isr138, INT_GATE);
    idt_register_entry(139, isr139, INT_GATE);
    idt_register_entry(140, isr140, INT_GATE);
    idt_register_entry(141, isr141, INT_GATE);
    idt_register_entry(142, isr142, INT_GATE);
    idt_register_entry(143, isr143, INT_GATE);
    idt_register_entry(144, isr144, INT_GATE);
    idt_register_entry(145, isr145, INT_GATE);
    idt_register_entry(146, isr146, INT_GATE);
    idt_register_entry(147, isr147, INT_GATE);
    idt_register_entry(148, isr148, INT_GATE);
    idt_register_entry(149, isr149, INT_GATE);
    idt_register_entry(150, isr150, INT_GATE);
    idt_register_entry(151, isr151, INT_GATE);
    idt_register_entry(152, isr152, INT_GATE);
    idt_register_entry(153, isr153, INT_GATE);
    idt_register_entry(154, isr154, INT_GATE);
    idt_register_entry(155, isr155, INT_GATE);
    idt_register_entry(156, isr156, INT_GATE);
    idt_register_entry(157, isr157, INT_GATE);
    idt_register_entry(158, isr158, INT_GATE);
    idt_register_entry(159, isr159, INT_GATE);
    idt_register_entry(160, isr160, INT_GATE);
    idt_register_entry(161, isr161, INT_GATE);
    idt_register_entry(162, isr162, INT_GATE);
    idt_register_entry(163, isr163, INT_GATE);
    idt_register_entry(164, isr164, INT_GATE);
    idt_register_entry(165, isr165, INT_GATE);
    idt_register_entry(166, isr166, INT_GATE);
    idt_register_entry(167, isr167, INT_GATE);
    idt_register_entry(168, isr168, INT_GATE);
    idt_register_entry(169, isr169, INT_GATE);
    idt_register_entry(170, isr170, INT_GATE);
    idt_register_entry(171, isr171, INT_GATE);
    idt_register_entry(172, isr172, INT_GATE);
    idt_register_entry(173, isr173, INT_GATE);
    idt_register_entry(174, isr174, INT_GATE);
    idt_register_entry(175, isr175, INT_GATE);
    idt_register_entry(176, isr176, INT_GATE);
    idt_register_entry(177, isr177, INT_GATE);
    idt_register_entry(178, isr178, INT_GATE);
    idt_register_entry(179, isr179, INT_GATE);
    idt_register_entry(180, isr180, INT_GATE);
    idt_register_entry(181, isr181, INT_GATE);
    idt_register_entry(182, isr182, INT_GATE);
    idt_register_entry(183, isr183, INT_GATE);
    idt_register_entry(184, isr184, INT_GATE);
    idt_register_entry(185, isr185, INT_GATE);
    idt_register_entry(186, isr186, INT_GATE);
    idt_register_entry(187, isr187, INT_GATE);
    idt_register_entry(188, isr188, INT_GATE);
    idt_register_entry(189, isr189, INT_GATE);
    idt_register_entry(190, isr190, INT_GATE);
    idt_register_entry(191, isr191, INT_GATE);
    idt_register_entry(192, isr192, INT_GATE);
    idt_register_entry(193, isr193, INT_GATE);
    idt_register_entry(194, isr194, INT_GATE);
    idt_register_entry(195, isr195, INT_GATE);
    idt_register_entry(196, isr196, INT_GATE);
    idt_register_entry(197, isr197, INT_GATE);
    idt_register_entry(198, isr198, INT_GATE);
    idt_register_entry(199, isr199, INT_GATE);
    idt_register_entry(200, isr200, INT_GATE);
    idt_register_entry(201, isr201, INT_GATE);
    idt_register_entry(202, isr202, INT_GATE);
    idt_register_entry(203, isr203, INT_GATE);
    idt_register_entry(204, isr204, INT_GATE);
    idt_register_entry(205, isr205, INT_GATE);
    idt_register_entry(206, isr206, INT_GATE);
    idt_register_entry(207, isr207, INT_GATE);
    idt_register_entry(208, isr208, INT_GATE);
    idt_register_entry(209, isr209, INT_GATE);
    idt_register_entry(210, isr210, INT_GATE);
    idt_register_entry(211, isr211, INT_GATE);
    idt_register_entry(212, isr212, INT_GATE);
    idt_register_entry(213, isr213, INT_GATE);
    idt_register_entry(214, isr214, INT_GATE);
    idt_register_entry(215, isr215, INT_GATE);
    idt_register_entry(216, isr216, INT_GATE);
    idt_register_entry(217, isr217, INT_GATE);
    idt_register_entry(218, isr218, INT_GATE);
    idt_register_entry(219, isr219, INT_GATE);
    idt_register_entry(220, isr220, INT_GATE);
    idt_register_entry(221, isr221, INT_GATE);
    idt_register_entry(222, isr222, INT_GATE);
    idt_register_entry(223, isr223, INT_GATE);
    idt_register_entry(224, isr224, INT_GATE);
    idt_register_entry(225, isr225, INT_GATE);
    idt_register_entry(226, isr226, INT_GATE);
    idt_register_entry(227, isr227, INT_GATE);
    idt_register_entry(228, isr228, INT_GATE);
    idt_register_entry(229, isr229, INT_GATE);
    idt_register_entry(230, isr230, INT_GATE);
    idt_register_entry(231, isr231, INT_GATE);
    idt_register_entry(232, isr232, INT_GATE);
    idt_register_entry(233, isr233, INT_GATE);
    idt_register_entry(234, isr234, INT_GATE);
    idt_register_entry(235, isr235, INT_GATE);
    idt_register_entry(236, isr236, INT_GATE);
    idt_register_entry(237, isr237, INT_GATE);
    idt_register_entry(238, isr238, INT_GATE);
    idt_register_entry(239, isr239, INT_GATE);
    idt_register_entry(240, isr240, INT_GATE);
    idt_register_entry(241, isr241, INT_GATE);
    idt_register_entry(242, isr242, INT_GATE);
    idt_register_entry(243, isr243, INT_GATE);
    idt_register_entry(244, isr244, INT_GATE);
    idt_register_entry(245, isr245, INT_GATE);
    idt_register_entry(246, isr246, INT_GATE);
    idt_register_entry(247, isr247, INT_GATE);
    idt_register_entry(248, isr248, INT_GATE);
    idt_register_entry(249, isr249, INT_GATE);
    idt_register_entry(250, isr250, INT_GATE);
    idt_register_entry(251, isr251, INT_GATE);
    idt_register_entry(252, isr252, INT_GATE);
    idt_register_entry(253, isr253, INT_GATE);
    idt_register_entry(254, isr254, INT_GATE);
    idt_register_entry(255, isr255, INT_GATE);
}

void
idt_init(void)
{
    idt_reload();
    idt_register_entries();
    asm volatile ("sti"); /* enable interrupts */
}
