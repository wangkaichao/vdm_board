
#ifndef _BR_CMD_TYPE_H
#define _BR_CMD_TYPE_H

//================================================================
#define T_Get	0
#define T_Set	1
#define T_Result 2
//.............其他命令类型设置

#define ENABLE_NO 0 //各属性值使能
#define ENABLE_YES 1

#define RESULT_OK 0 //成功
#define RESULT_ERROR 1 //普通错误
#define RESULT_OTHER 2 //其他错误
//......................各错误说明
#define MSG_LENGTH 512
enum{
	e_TYPE = 0,

	//Av_cfg_t
	e_Chn,
	e_Sub_Chn,
	e_FW,
	e_denoise,
	e_input_system,
	e_de_interlace,
	e_Stream_enable,
	e_enc_type,
	e_frame_rate,
	e_bit_rate,
	e_ip_interval,
	e_width,
	e_height,
	e_rate_ctl_type,
	e_target_rate_max,
	e_reaction_delay_max,
	e_init_quant,
	e_max_quant,
	e_min_quant,
	e_mjpeg_quality,
	e_enable_roi,
	e_roi_x,
	e_roi_y,
	e_roi_w,
	e_roi_h,
	//Img_cfg_t
	e_brightness,
	e_saturation,
	e_contrast,
	e_hue,
	e_scene,
	e_flip,
	e_mirror,
	e_imgctow,
	e_imgwtoc,
	e_wb_r,
	e_wb_g,
	e_wb_b,
	//Osd_cfg_t
	e_osd_region,
	e_osd_enable,
	e_osd_x,
	e_osd_y,
	e_osd_w,
	e_osd_h,
	e_osd_region_name,
	//Infrared_cfg_t
	e_infrad_stat,
	//Net_cfg_t
	e_net_dhcpflag,
	e_net_ip,
	e_net_netmask,
	e_net_gateway,
	e_net_dnsstat,
	e_net_fdnsip,
	e_net_sdnsip,
	e_net_macaddr,
	e_net_nettype,
	//Port_cfg_t
	e_port_httpport,
	e_port_rtspport,
	//Upnp_cfg_t
	e_upnp_upmenable,
	//DDNS_cfg_t	
	e_ddns_d3thenable,
	e_ddns_d3thservice,
	e_ddns_d3thuname,
	e_ddns_d3thpasswd,
	e_ddns_domain,
	//Wf_cfg_t
	e_wf_enable,
	e_wf_ssid,
	e_wf_auth,
	e_wf_key,
	e_wf_enc,
	e_wf_mode,
	e_wfsearch_accesspoints,
	e_wfsearch_channel,
	e_wfsearch_rssi,
	e_wfsearch_essid,
	e_wfsearch_enc,
	e_wfsearch_auth,
	e_wfsearch_net,
	//Ptz_cfg_t
	e_ptz_protocal,
	e_ptz_address,
	e_ptz_baud,
	e_ptz_databit,
	e_ptz_stopbit,
	e_ptz_check,
	//Md_cfg_t
	e_md_io_alarm_enable,
	e_md_io_alarm_flag,
	e_md_email_switch,
	e_md_snap_switch,
	e_md_record_switch,
	e_md_ftp_switch,
	e_md_relay_switch,
	e_md_relay_time,
	e_md_schedule_type,
	e_md_schedule_ename,
	e_md_schedule_etm,
	e_md_schedule_workday_Tstart,
	e_md_schedule_workday_Tend,
	e_md_schedule_workend_Tstart,
	e_md_schedule_workend_Tend,
	e_md_schedule_week_Tstart,
	e_md_schedule_week_Tend,
	e_md_area,
	e_md_area_eable,
	e_md_area_s,
	e_md_area_method,
	e_md_area_x,
	e_md_area_y,
	e_md_area_w,
	e_md_area_h,
	//Snaptimer_cfg_t
	e_snapT_enable,
	e_snapT_interval,
	e_snapT_type,
	//Usr_cfg_t
	e_user_number,
	e_user_opt_type,	//对user操作方式：添加，删除，修改密码
	e_user_name,
	e_user_password,
	e_user_group,
	//Ft_cfg_t
	e_ft_serverip,
	e_ft_port,
	e_ft_username,
	e_ft_password,
	e_ft_mode,
	e_ft_dirname,
	//Smtp_cfg_t
	e_smtp_serverip,
	e_smtp_port,
	e_smtp_sslflag,
	e_smtp_logintype,
	e_smtp_username,
	e_smtp_password,
	e_smtp_from,
	e_smtp_to,
	e_smtp_subject,
	e_smtp_text,
	//VidMask_cfg_t
	e_vdmask_number,
	e_vdmask_NO,
	e_vdmask_enable,
	e_vdmask_x,
	e_vdmask_y,
	e_vdmask_w,
	e_vdmask_h,
	e_vdmask_color,
	//Time_cfg_t
	e_time_Zone,
	e_time_dstmode,
	e_time_ntpenable,
	e_time_ntpserver,
	e_time_ntpinterval,
	e_time_systime,
	//Sys_cfg_t
	e_sys_devtype,
	e_sys_model,
	e_sys_hdversion,
	e_sys_swversion,
	e_sys_devname,
	e_sys_startdate,
	e_sys_runtimes,
	e_sys_sdstatus,
	e_sys_sdfreespace,
	e_sys_sdtotalspace,
	//to NVR
	e_nvr_opt,	//1:开始获取视频数据 0：停止视频数据    //channel opt channel opt
	e_nvr_forIDR,	//1:取一IDR帧
	e_nvr_clientID,
	e_END
};

typedef struct _tagMem_head{
	char	channel;	//0:main stream  1:sub stream
//	char	is_I_frame;	//1:I frame  0:not I frame
	char	frame_type;// frame type: H264NALTYPE
	char	statue;		//memory current status: 1: have new data 0:data is old
	//char	reserved;
	char frame_index;	


	int		width;	//image width
	int		height;	//image height
	int		size;
}Mem_head;

enum H264NALTYPE{
	H264NT_NAL = 0,
	H264NT_SLICE,
	H264NT_SLICE_DPA,
	H264NT_SLICE_DPB,
	H264NT_SLICE_DPC,
	H264NT_SLICE_IDR,
	H264NT_SEI,
	H264NT_SPS,
	H264NT_PPS
};



#define TO_NVR_MEM_KEY0		0x12340000
#define TO_NVR_MEM_KEY1 	0x12340101
#define TO_NVR_MEM_KEY2		0x12340202
#define TO_NVR_MEM_KEY3 	0x12340303
#define TO_NVR_MEM_KEY4		0x12340404
#define TO_NVR_MEM_KEY5 	0x12340505
#define TO_NVR_MEM_KEY6		0x12340606
#define TO_NVR_MEM_KEY7 	0x12340707
#define TO_NVR_MEM_KEY8		0x12340808
#define TO_NVR_MEM_KEY9		0x12340909
#define TO_NVR_MEM_KEYA		0x12340a0a
#define TO_NVR_MEM_KEYB		0x12340b0b
#define TO_NVR_MEM_KEYC		0x12340c0c
#define TO_NVR_MEM_KEYD		0x12340d0d
#define TO_NVR_MEM_KEYE		0x12340e0e
#define TO_NVR_MEM_KEYF		0x12340f0f
#define TO_NVR_MEM_KEY10	0x12341010
#define TO_NVR_MEM_KEY11 	0x12341111
#define TO_NVR_MEM_KEY12	0x12341212
#define TO_NVR_MEM_KEY13 	0x12341313


#define TO_NVR_SEM_KEY0		0x23450000
#define TO_NVR_SEM_KEY1		0x23450101
#define TO_NVR_SEM_KEY2		0x23450202
#define TO_NVR_SEM_KEY3		0x23450303
#define TO_NVR_SEM_KEY4		0x23450404
#define TO_NVR_SEM_KEY5		0x23450505
#define TO_NVR_SEM_KEY6		0x23450606
#define TO_NVR_SEM_KEY7		0x23450707
#define TO_NVR_SEM_KEY8		0x23450808
#define TO_NVR_SEM_KEY9		0x23450909
#define TO_NVR_SEM_KEYA		0x23450a0a
#define TO_NVR_SEM_KEYB		0x23450b0b
#define TO_NVR_SEM_KEYC		0x23450c0c
#define TO_NVR_SEM_KEYD		0x23450d0d
#define TO_NVR_SEM_KEYE		0x23450e0e
#define TO_NVR_SEM_KEYF		0x23450f0f
#define TO_NVR_SEM_KEY10	0x23451010
#define TO_NVR_SEM_KEY11	0x23451111
#define TO_NVR_SEM_KEY12	0x23451212
#define TO_NVR_SEM_KEY13	0x23451313

#endif
