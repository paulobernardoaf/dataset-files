#define R_MIG_INDEX_LEN 846
static const char *mig_index[R_MIG_INDEX_LEN] = {
"65", "mach_notify_port_deleted",
"69", "mach_notify_port_destroyed",
"70", "mach_notify_no_senders",
"71", "mach_notify_send_once",
"72", "mach_notify_dead_name",
"123", "audit_triggers",
"200", "host_info",
"201", "host_kernel_version",
"202", "host_page_size",
"203", "mach_memory_object_memory_entry",
"204", "host_processor_info",
"205", "host_get_io_master",
"206", "host_get_clock_service",
"207", "kmod_get_info",
"209", "host_virtual_physical_table_info",
"210", "host_ipc_hash_info",
"211", "enable_bluebox",
"212", "disable_bluebox",
"213", "processor_set_default",
"214", "processor_set_create",
"215", "mach_memory_object_memory_entry_64",
"216", "host_statistics",
"217", "host_request_notification",
"218", "host_lockgroup_info",
"219", "host_statistics64",
"220", "mach_zone_info",
"221", "mach_zone_force_gc",
"222", "_kernelrpc_host_create_mach_voucher",
"223", "host_register_mach_voucher_attr_manager",
"224", "host_register_well_known_mach_voucher_attr_manager",
"225", "host_set_atm_diagnostic_flag",
"227", "mach_memory_info",
"228", "host_set_multiuser_config_flags",
"231", "mach_zone_info_for_zone",
"232", "mach_zone_info_for_largest_zone",
"233", "mach_zone_get_zlog_zones",
"234", "mach_zone_get_btlog_records",
"400", "host_get_boot_info",
"401", "host_reboot",
"402", "host_priv_statistics",
"403", "host_default_memory_manager",
"404", "vm_wire",
"405", "thread_wire",
"406", "vm_allocate_cpm",
"407", "host_processors",
"408", "host_get_clock_control",
"409", "kmod_create",
"410", "kmod_destroy",
"411", "kmod_control",
"412", "host_get_special_port",
"413", "host_set_special_port",
"414", "host_set_exception_ports",
"415", "host_get_exception_ports",
"416", "host_swap_exception_ports",
"417", "host_load_symbol_table",
"418", "mach_vm_wire",
"419", "host_processor_sets",
"420", "host_processor_set_priv",
"421", "set_dp_control_port",
"422", "get_dp_control_port",
"423", "host_set_UNDServer",
"424", "host_get_UNDServer",
"425", "kext_request",
"600", "host_security_create_task_token",
"601", "host_security_set_task_token",
"999", "mach_gss_init_sec_context",
"1000", "clock_get_time",
"1001", "clock_get_attributes",
"1002", "clock_alarm",
"1003", "mach_gss_accept_sec_context_v2",
"1004", "mach_gss_hold_cred",
"1005", "mach_gss_unhold_cred",
"1023", "lockd_request",
"1024", "lockd_ping",
"1025", "lockd_shutdown",
"1040", "netname_check_in",
"1041", "netname_look_up",
"1042", "netname_check_out",
"1043", "netname_version",
"1200", "clock_set_time",
"1201", "clock_set_attributes",
"2000", "memory_object_get_attributes",
"2001", "memory_object_change_attributes",
"2002", "memory_object_synchronize_completed",
"2003", "memory_object_lock_request",
"2004", "memory_object_destroy",
"2005", "memory_object_upl_request",
"2006", "memory_object_super_upl_request",
"2007", "memory_object_cluster_size",
"2008", "memory_object_page_op",
"2009", "memory_object_recover_named",
"2010", "memory_object_release_name",
"2011", "memory_object_range_op",
"2050", "upl_abort",
"2051", "upl_abort_range",
"2052", "upl_commit",
"2053", "upl_commit_range",
"2200", "memory_object_init",
"2201", "memory_object_terminate",
"2202", "memory_object_data_request",
"2203", "memory_object_data_return",
"2204", "memory_object_data_initialize",
"2205", "memory_object_data_unlock",
"2206", "memory_object_synchronize",
"2207", "memory_object_map",
"2208", "memory_object_last_unmap",
"2209", "memory_object_data_reclaim",
"2250", "memory_object_create",
"2275", "default_pager_object_create",
"2276", "default_pager_info",
"2277", "default_pager_objects",
"2278", "default_pager_object_pages",
"2280", "default_pager_backing_store_create",
"2281", "default_pager_backing_store_delete",
"2282", "default_pager_backing_store_info",
"2283", "default_pager_add_file",
"2284", "default_pager_triggers",
"2285", "default_pager_info_64",
"2295", "default_pager_space_alert",
"2401", "exception_raise",
"2402", "exception_raise_state",
"2403", "exception_raise_state_identity",
"2405", "mach_exception_raise",
"2406", "mach_exception_raise_state",
"2407", "mach_exception_raise_state_identity",
"2800", "io_object_get_class",
"2801", "io_object_conforms_to",
"2802", "io_iterator_next",
"2803", "io_iterator_reset",
"2804", "io_service_get_matching_services",
"2805", "io_registry_entry_get_property",
"2806", "io_registry_create_iterator",
"2807", "io_registry_iterator_enter_entry",
"2808", "io_registry_iterator_exit_entry",
"2809", "io_registry_entry_from_path",
"2810", "io_registry_entry_get_name",
"2811", "io_registry_entry_get_properties",
"2812", "io_registry_entry_get_property_bytes",
"2813", "io_registry_entry_get_child_iterator",
"2814", "io_registry_entry_get_parent_iterator",
"2816", "io_service_close",
"2817", "io_connect_get_service",
"2818", "io_connect_set_notification_port",
"2819", "io_connect_map_memory",
"2820", "io_connect_add_client",
"2821", "io_connect_set_properties",
"2822", "io_connect_method_scalarI_scalarO",
"2823", "io_connect_method_scalarI_structureO",
"2824", "io_connect_method_scalarI_structureI",
"2825", "io_connect_method_structureI_structureO",
"2826", "io_registry_entry_get_path",
"2827", "io_registry_get_root_entry",
"2828", "io_registry_entry_set_properties",
"2829", "io_registry_entry_in_plane",
"2830", "io_object_get_retain_count",
"2831", "io_service_get_busy_state",
"2832", "io_service_wait_quiet",
"2833", "io_registry_entry_create_iterator",
"2834", "io_iterator_is_valid",
"2836", "io_catalog_send_data",
"2837", "io_catalog_terminate",
"2838", "io_catalog_get_data",
"2839", "io_catalog_get_gen_count",
"2840", "io_catalog_module_loaded",
"2841", "io_catalog_reset",
"2842", "io_service_request_probe",
"2843", "io_registry_entry_get_name_in_plane",
"2844", "io_service_match_property_table",
"2845", "io_async_method_scalarI_scalarO",
"2846", "io_async_method_scalarI_structureO",
"2847", "io_async_method_scalarI_structureI",
"2848", "io_async_method_structureI_structureO",
"2849", "io_service_add_notification",
"2850", "io_service_add_interest_notification",
"2851", "io_service_acknowledge_notification",
"2852", "io_connect_get_notification_semaphore",
"2853", "io_connect_unmap_memory",
"2854", "io_registry_entry_get_location_in_plane",
"2855", "io_registry_entry_get_property_recursively",
"2856", "io_service_get_state",
"2857", "io_service_get_matching_services_ool",
"2858", "io_service_match_property_table_ool",
"2859", "io_service_add_notification_ool",
"2860", "io_object_get_superclass",
"2861", "io_object_get_bundle_identifier",
"2862", "io_service_open_extended",
"2863", "io_connect_map_memory_into_task",
"2864", "io_connect_unmap_memory_from_task",
"2865", "io_connect_method",
"2866", "io_connect_async_method",
"2867", "io_connect_set_notification_port_64",
"2868", "io_service_add_notification_64",
"2869", "io_service_add_interest_notification_64",
"2870", "io_service_add_notification_ool_64",
"2871", "io_registry_entry_get_registry_entry_id",
"3000", "processor_start",
"3001", "processor_exit",
"3002", "processor_info",
"3003", "processor_control",
"3004", "processor_assign",
"3005", "processor_get_assignment",
"3200", "mach_port_names",
"3201", "mach_port_type",
"3202", "mach_port_rename",
"3203", "mach_port_allocate_name",
"3204", "mach_port_allocate",
"3205", "mach_port_destroy",
"3206", "mach_port_deallocate",
"3207", "mach_port_get_refs",
"3208", "mach_port_mod_refs",
"3209", "_kernelrpc_mach_port_peek",
"3210", "mach_port_set_mscount",
"3211", "mach_port_get_set_status",
"3212", "mach_port_move_member",
"3213", "mach_port_request_notification",
"3214", "mach_port_insert_right",
"3215", "mach_port_extract_right",
"3216", "mach_port_set_seqno",
"3217", "mach_port_get_attributes",
"3218", "mach_port_set_attributes",
"3219", "mach_port_allocate_qos",
"3220", "mach_port_allocate_full",
"3221", "task_set_port_space",
"3222", "mach_port_get_srights",
"3223", "mach_port_space_info",
"3224", "mach_port_dnrequest_info",
"3225", "mach_port_kernel_object",
"3226", "mach_port_insert_member",
"3227", "mach_port_extract_member",
"3228", "mach_port_get_context",
"3229", "mach_port_set_context",
"3230", "mach_port_kobject",
"3231", "_kernelrpc_mach_port_construct",
"3232", "_kernelrpc_mach_port_destruct",
"3233", "_kernelrpc_mach_port_guard",
"3234", "_kernelrpc_mach_port_unguard",
"3235", "_kernelrpc_mach_port_space_basic_info",
"3236", "_kernelrpc_mach_port_special_reply_port_reset_link",
"3400", "task_create",
"3401", "task_terminate",
"3402", "task_threads",
"3403", "mach_ports_register",
"3404", "mach_ports_lookup",
"3405", "task_info",
"3406", "task_set_info",
"3407", "task_suspend",
"3408", "task_resume",
"3409", "task_get_special_port",
"3410", "task_set_special_port",
"3411", "thread_create",
"3412", "thread_create_running",
"3413", "task_set_exception_ports",
"3414", "task_get_exception_ports",
"3415", "task_swap_exception_ports",
"3416", "lock_set_create",
"3417", "lock_set_destroy",
"3418", "semaphore_create",
"3419", "semaphore_destroy",
"3420", "task_policy_set",
"3421", "task_policy_get",
"3422", "task_sample",
"3423", "task_policy",
"3424", "task_set_emulation",
"3425", "task_get_emulation_vector",
"3426", "task_set_emulation_vector",
"3427", "task_set_ras_pc",
"3428", "task_zone_info",
"3429", "task_assign",
"3430", "task_assign_default",
"3431", "task_get_assignment",
"3432", "task_set_policy",
"3433", "task_get_state",
"3434", "task_set_state",
"3435", "task_set_phys_footprint_limit",
"3436", "task_suspend2",
"3437", "task_resume2",
"3438", "task_purgable_info",
"3439", "task_get_mach_voucher",
"3440", "task_set_mach_voucher",
"3441", "task_swap_mach_voucher",
"3442", "task_generate_corpse",
"3443", "task_map_corpse_info",
"3444", "task_register_dyld_image_infos",
"3445", "task_unregister_dyld_image_infos",
"3446", "task_get_dyld_image_infos",
"3447", "task_register_dyld_shared_cache_image_info",
"3448", "task_register_dyld_set_dyld_state",
"3449", "task_register_dyld_get_process_state",
"3450", "task_map_corpse_info_64",
"3451", "task_inspect",
"3600", "thread_terminate",
"3601", "act_get_state",
"3602", "act_set_state",
"3603", "thread_get_state",
"3604", "thread_set_state",
"3605", "thread_suspend",
"3606", "thread_resume",
"3607", "thread_abort",
"3608", "thread_abort_safely",
"3609", "thread_depress_abort",
"3610", "thread_get_special_port",
"3611", "thread_set_special_port",
"3612", "thread_info",
"3613", "thread_set_exception_ports",
"3614", "thread_get_exception_ports",
"3615", "thread_swap_exception_ports",
"3616", "thread_policy",
"3617", "thread_policy_set",
"3618", "thread_policy_get",
"3619", "thread_sample",
"3620", "etap_trace_thread",
"3621", "thread_assign",
"3622", "thread_assign_default",
"3623", "thread_get_assignment",
"3624", "thread_set_policy",
"3625", "thread_get_mach_voucher",
"3626", "thread_set_mach_voucher",
"3627", "thread_swap_mach_voucher",
"3800", "vm_region",
"3801", "vm_allocate",
"3802", "vm_deallocate",
"3803", "vm_protect",
"3804", "vm_inherit",
"3805", "vm_read",
"3806", "vm_read_list",
"3807", "vm_write",
"3808", "vm_copy",
"3809", "vm_read_overwrite",
"3810", "vm_msync",
"3811", "vm_behavior_set",
"3812", "vm_map",
"3813", "vm_machine_attribute",
"3814", "vm_remap",
"3815", "task_wire",
"3816", "mach_make_memory_entry",
"3817", "vm_map_page_query",
"3818", "mach_vm_region_info",
"3819", "vm_mapped_pages_info",
"3821", "vm_region_recurse",
"3822", "vm_region_recurse_64",
"3823", "mach_vm_region_info_64",
"3824", "vm_region_64",
"3825", "mach_make_memory_entry_64",
"3826", "vm_map_64",
"3827", "vm_map_get_upl",
"3830", "vm_purgable_control",
"4000", "processor_set_statistics",
"4001", "processor_set_destroy",
"4002", "processor_set_max_priority",
"4003", "processor_set_policy_enable",
"4004", "processor_set_policy_disable",
"4005", "processor_set_tasks",
"4006", "processor_set_threads",
"4007", "processor_set_policy_control",
"4008", "processor_set_stack_usage",
"4009", "processor_set_info",
"4800", "mach_vm_allocate",
"4801", "mach_vm_deallocate",
"4802", "mach_vm_protect",
"4803", "mach_vm_inherit",
"4804", "mach_vm_read",
"4805", "mach_vm_read_list",
"4806", "mach_vm_write",
"4807", "mach_vm_copy",
"4808", "mach_vm_read_overwrite",
"4809", "mach_vm_msync",
"4810", "mach_vm_behavior_set",
"4811", "mach_vm_map",
"4812", "mach_vm_machine_attribute",
"4813", "mach_vm_remap",
"4814", "mach_vm_page_query",
"4815", "mach_vm_region_recurse",
"4816", "mach_vm_region",
"4817", "_mach_make_memory_entry",
"4818", "mach_vm_purgable_control",
"4819", "mach_vm_page_info",
"4820", "mach_vm_page_range_query",
"5000", "ledger_create",
"5001", "ledger_terminate",
"5002", "ledger_transfer",
"5003", "ledger_read",
"5200", "mach_get_task_label",
"5201", "mach_get_task_label_text",
"5202", "mach_get_label",
"5203", "mach_get_label_text",
"5204", "mach_set_port_label",
"5205", "mac_check_service",
"5206", "mac_port_check_service_obj",
"5207", "mac_port_check_access",
"5208", "mac_label_new",
"5209", "mac_request_label",
"5400", "mach_voucher_extract_attr_content",
"5401", "_kernelrpc_mach_voucher_extract_attr_recipe",
"5402", "mach_voucher_extract_all_attr_recipes",
"5403", "mach_voucher_attr_command",
"5404", "mach_voucher_debug_info",
"6000", "UNDExecute_rpc",
"6001", "UNDDisplayNoticeFromBundle_rpc",
"6002", "UNDDisplayAlertFromBundle_rpc",
"6003", "UNDDisplayCustomFromBundle_rpc",
"6004", "UNDDisplayCustomFromDictionary_rpc",
"6005", "UNDCancelNotification_rpc",
"6006", "UNDDisplayNoticeSimple_rpc",
"6007", "UNDDisplayAlertSimple_rpc",
"6200", "UNDAlertCompletedWithResult_rpc",
"6201", "UNDNotificationCreated_rpc",
"27000", "check_task_access",
"27001", "find_code_signature",
"77000", "kextd_ping",
"617000", "lock_acquire",
"617001", "lock_release",
"617002", "lock_try",
"617003", "lock_make_stable",
"617004", "lock_handoff",
"617005", "lock_handoff_accept",
"617200", "semaphore_signal",
"617201", "semaphore_signal_all",
"617202", "semaphore_wait",
"617203", "semaphore_signal_thread",
"617204", "semaphore_timedwait",
"617205", "semaphore_wait_signal",
"617206", "semaphore_timedwait_signal",
"3125107", "clock_alarm_reply",
};
