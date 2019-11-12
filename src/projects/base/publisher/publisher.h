//==============================================================================
//
//  OvenMediaEngine
//
//  Created by Getroot
//  Copyright (c) 2018 AirenSoft. All rights reserved.
//
//==============================================================================
#pragma once

#include <base/common_types.h>
#include <base/info/host.h>
#include <base/media_route/media_route_application_observer.h>
#include <base/ovcrypto/ovcrypto.h>
#include <base/publisher/application.h>
#include <base/publisher/stream.h>

#include <modules/ice/ice_port_manager.h>
#include <modules/physical_port/physical_port.h>

#include <chrono>

//====================================================================================================
// Monitoring Collect Data
//====================================================================================================
enum class MonitroingCollectionType
{
	Stream = 0,
	App,
	Origin,
	Host,
};

struct MonitoringCollectionData
{
	MonitoringCollectionData() = default;

	MonitoringCollectionData(MonitroingCollectionType type_,
							 const ov::String &origin_name_ = "",
							 const ov::String &app_name_ = "",
							 const ov::String &stream_name_ = "")
	{
		type = type_;
		type_string = GetTypeString(type);
		origin_name = origin_name_;
		app_name = app_name_;
		stream_name = stream_name_;
	}

	void Append(const std::shared_ptr<MonitoringCollectionData> &collection)
	{
		edge_connection += collection->edge_connection;
		edge_bitrate += collection->edge_bitrate;
		p2p_connection += collection->p2p_connection;
		p2p_bitrate += collection->p2p_bitrate;
	}

	static ov::String GetTypeString(MonitroingCollectionType type)
	{
		ov::String result;

		if (type == MonitroingCollectionType::Stream)
			result = "stream";
		else if (type == MonitroingCollectionType::App)
			result = "app";
		else if (type == MonitroingCollectionType::Origin)
			result = "org";
		else if (type == MonitroingCollectionType::Host)
			result = "host";

		return result;
	}

	MonitroingCollectionType type = MonitroingCollectionType::Stream;
	ov::String type_string;
	ov::String origin_name;
	ov::String app_name;
	ov::String stream_name;
	uint32_t edge_connection = 0;					   // count
	uint64_t edge_bitrate = 0;						   // bps
	uint32_t p2p_connection = 0;					   // count
	uint64_t p2p_bitrate = 0;						   // bps
	std::chrono::system_clock::time_point check_time;  // (chrono)
};

// All publishers such as WebRTC, HLS and MPEG-DASH has to inherit the Publisher class and implement that interfaces
class Publisher : public MediaRouteObserver
{
public:
	virtual bool Start();
	virtual bool Stop();

	std::shared_ptr<Application> GetApplicationByName(ov::String app_name);
	std::shared_ptr<Stream> GetStream(ov::String app_name, ov::String stream_name);
	template <typename T>
	std::shared_ptr<T> GetStreamAs(ov::String app_name, ov::String stream_name)
	{
		return std::static_pointer_cast<T>(GetStream(app_name, stream_name));
	}

	std::shared_ptr<Application> GetApplicationById(info::application_id_t application_id);
	std::shared_ptr<Stream> GetStream(info::application_id_t application_id, uint32_t stream_id);
	template <typename T>
	std::shared_ptr<T> GetStreamAs(info::application_id_t application_id, uint32_t stream_id)
	{
		return std::static_pointer_cast<T>(GetStream(application_id, stream_id));
	}

	// monitoring data pure virtual function
	// - collected_datas vector must be insert processed
	virtual bool GetMonitoringCollectionData(std::vector<std::shared_ptr<MonitoringCollectionData>> &collections) = 0;


	///////////////////////////////////////
	// Implement MediaRouteObserver
	///////////////////////////////////////
	// Create Application
	bool OnCreateApplication(const info::Application &app_info) override;

	// Delete Application
	bool OnDeleteApplication(const info::Application &app_info) override;

protected:
	explicit Publisher(const info::Host &host_info, const std::shared_ptr<MediaRouteInterface> &router);
	virtual ~Publisher() = default;

	// Host Info
	const info::Host& GetHostInfo();

	// Each Publisher should define their type
	virtual cfg::PublisherType GetPublisherType() const = 0;
	virtual const char *GetPublisherName() const = 0;
	virtual std::shared_ptr<Application> OnCreatePublisherApplication(const info::Application &application_info) = 0;

	std::map<info::application_id_t, std::shared_ptr<Application>> _applications;

	const info::Host _host_info;
	std::shared_ptr<MediaRouteInterface> _router;
};
