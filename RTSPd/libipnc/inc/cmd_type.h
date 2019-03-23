
#ifndef _BR_CMD_TYPE_H
#define _BR_CMD_TYPE_H

//================================================================
#define MAX_CMD_LEN		512
#define MSG_LENGTH MAX_CMD_LEN
#define T_Get	0
#define T_Set	1
#define T_Result 2
//.............����������������

#define ENABLE_NO 0 //������ֵʹ��
#define ENABLE_YES 1

#define RESULT_OK 0 //�ɹ�
#define RESULT_ERROR 1 //��ͨ����
#define RESULT_OTHER 2 //��������

//PTZ error NO
#define ERROR_PTZ_INDEX_NOT_EXIST	40	//preset index not exist
#define ERROR_PTZ_INDEX_OUT_RANGE	41	//preset index out range
#define ERROR_PTZ_PRESETNAME_NOT_EXIST	42	//presetname not exist
#define ERROR_PTZ_INDEX_OVER	43
#define ERROR_PTZ_PRESETNAME_EMPTY	44
#define ERROR_PTZ_PRESETNAME_REPEAT	45
//......................������˵��

#define ADD_USER	0
#define DEL_USER	1
#define MODIFY_USER	2

enum{
	e_TYPE = 0,

	//Av_cfg_t
	e_Chn,
	e_Sub_Chn,
	//======for onvif==========
	e_video_chn_num,	
	e_video_addr,
	
	e_encode_profile,//for h264 profile level 0:H264 baseline
	e_profile_levels,//bit0:baseline bit1:main profile bit2:high profile
	e_video_SynchronizationPoint, //insert I frame
	//=========================
	
	e_FW,
	e_denoise,
	e_input_system,
	e_de_interlace,
	e_Stream_enable,
	e_enc_type,
	e_frame_rate,
	//===============for onvif=================
	e_min_frame_rate,
	e_max_frame_rate,
	//=========================================
	e_bit_rate,
	e_ip_interval,
	//===============for onvif=================
	e_min_ip_interval,
	e_max_ip_interval,
	//=========================================	
	e_width,
	e_height,
	//===============for onvif=================
	e_videoResolutions,//chn0:1920,1072/1280,720 chn1:352*288
	e_min_videoEncodingInterval,
	e_max_videoEncodingInterval,
	//=========================================	
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
	//========for onvif=============
	e_sharpness,
	//==============================
	e_hue,
	e_scene,
	e_flip,
	e_mirror,
	e_imgctow,
	e_imgwtoc,
	e_wb_r,
	e_wb_g,
	e_wb_b,
	//========for onvif============
	e_wb_mode,
	e_wb_crgain,
	e_wb_cbgain,
	
	e_backlightcomp_mode, //0 is off 1 is on
	e_backlightcomp_level, 
	
	e_wdrange_mode,  // 0 is off 1 is on
	e_wdrange_level,
	//=============================
	
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
	//==============for onvif=========
	e_net_cardname,
	e_net_protocols,
	//================================

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
	e_user_opt_type,	//��user������ʽ����ӣ�ɾ�����޸�����		0:add 1:del 2:modify
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
	//==========for onvif============
	e_sys_hardwareId,
	e_sys_manufacturer,
	e_sys_Model,
	e_sys_serialNumber,
	//===============================

	//to NVR
	e_nvr_opt,	// 1:��ʼ��ȡ��Ƶ���� 0��ֹͣ��Ƶ����
	e_nvr_forIDR,	// 1:ȡһIDR֡
	e_nvr_clientID,

	e_enc_resolution,	//�ֱ��ʣ�1-> 1080  0-> 720
	e_reset,			//�ָ���ʼ����

	//for onvif
	e_reboot,
	//==================
	
	e_mctp_ptzstring,

	//========for onvif ptz control==================
	e_ptz_continue_move_default_timeout,
	e_ptz_continue_move_vx,
	e_ptz_continue_move_vy,
	e_ptz_continue_move_vzoom,
	e_ptz_continue_move_timeout,
	e_ptz_stop_pt,
	e_ptz_stop_zoom,
	
	e_ptz_goto_preset,//����Ԥ�õ�λ��
	e_ptz_preset,//����Ԥ�õ�
	e_ptz_presetname,//Ԥ��λ���� ���32�ֽ�
	e_ptz_deletepreset,//
	e_ptz_allpresets,//�õ�����Ԥ�õ�����
	e_ptz_presets_capacity,//ϵͳ֧��Ԥ�õ����
	//======================================

	//=============for onvif audio======================
	e_audio_enable,//����Ƶͨ��ʹ�ܣ�
	e_audio_enc_type,//(��Ƶ���ͣ�
	e_audio_bitrate,//(��ƵBitrate)
	e_audio_rtspport,//(��Ƶrtspport)
//	e_audio_chn_num,// (��Ƶͨ��������
//	e_audio_Chn,//����Ƶͨ���ţ�
	e_audio_samplesize,
	e_audio_samplerate,
	//===================================================

	e_sys_protocol,

	e_error,
	
	e_snapshot_start,

	e_cld_enable,
	e_cld_id,
	e_cld_pw,
	e_cld_srv_prt,
	e_cld_srv_addr,
	e_cld_dev_name,

	//==========¼�����==========
	e_rec_enable,//ʹ��
	e_rec_len,//ÿ��¼��ʱ����1-5����
	e_rec_time,//¼��ʱ��Σ�24bit��ʾ��bit0�� 0��-1�㣬bit1�� 1��-2�� ������

	e_END
};

//==========================================================================

typedef struct _tagKey{
	int mem_key;
	int mid;
	int sem_key;
	int sid;
	char *pbuf;
	struct _tagKey *pNext;
}Key_t;

typedef struct _tagMem_head{
	char	channel;	//0:main stream  1:sub stream
//	char	is_I_frame;	//1:I frame  0:not I frame
	char	frame_type;// frame type: H264NALTYPE
	char	statue;		//memory current status: 1: have new data 0:data is old
//	char	reserved;
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
//==========================================================================

#endif
