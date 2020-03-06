#pragma once

#include <memory>
#include <vector>

#ifdef HOST_PYTHON_MODULE
#include <pybind11/pybind11.h>
#endif

#include "tensor_info.hpp"
#include "tensor_entry.hpp"
#include "../host_data_packet.hpp"


class TensorEntryContainer
{
public:
    TensorEntryContainer(
              std::vector<std::shared_ptr<HostDataPacket>> &tensors_raw_data,
        const std::vector<TensorInfo>                      &tensors_info,
              DepthCalculationInterface*                    depth_calculation_interface
    )
        : _tensors_raw_data(tensors_raw_data)
        , _tensors_info(&tensors_info)
        , _depth_calculation_interface(depth_calculation_interface)
    {}

    unsigned size() const
    {
        assert(nullptr != _tensors_info);
        assert(!_tensors_info->empty());
        unsigned number = (*_tensors_info)[0].getEntriesNumber();

        for (size_t i = 1; i < _tensors_info->size(); ++i)
        {
            assert((*_tensors_info)[i].getEntriesNumber() == number);
        }

        return number;
    }

    std::vector<TensorEntry> getByIndex(unsigned entry_index)
    {
        assert(nullptr != _tensors_info);
        // assert(nullptr != _tensors_raw_data);
        assert(_tensors_info->size() == _tensors_raw_data.size());

        std::vector<TensorEntry> entry;

        for (size_t tensor_index = 0; tensor_index < _tensors_info->size(); ++tensor_index)
        {
            TensorEntry te;

            const TensorInfo& ti = (*_tensors_info)[tensor_index];
            const auto& trd = _tensors_raw_data[tensor_index];

            auto entry_byte_size = ti.getEntryByteSize();

            te.raw_data = trd->data.data() + entry_index * entry_byte_size; // TODO: check whether it works for all outputs
            te.output_properties_type = ti.output_properties_type;
            te.output_properties_type_size = size_of_type(te.output_properties_type);
            te.properties_number = entry_byte_size / te.output_properties_type_size;
            te.nnet_input_width  = ti.nnet_input_width;
            te.nnet_input_height = ti.nnet_input_height;
            te.depth_calulator = _depth_calculation_interface;

            if (ti.output_properties_dimensions.size() == 1)
            {
                auto index = ti.output_properties_dimensions[0];

                if (!ti.output_property_key_string_to_index.empty() &&
                    (ti.output_property_key_string_to_index.size() > index))
                {
                    te.output_property_key_string_to_index = &ti.output_property_key_string_to_index.at(index);
                }

                // if ((ti.output_property_value_string_to_index.size() > ti.output_properties_dimensions[0]) &&
                //     !ti.output_property_value_string_to_index[ti.output_properties_dimensions[0]].empty())
                // {
                //     te.output_property_value_string_to_index =
                //             &ti.output_property_value_string_to_index[ti.output_properties_dimensions[0]];
                // }
            }

            entry.push_back(te);
        }

        return entry;
    }

    float getByName(const std::string &property_name)
    {
        return 0.1f;
    }


private:
          std::vector<std::shared_ptr<HostDataPacket>> _tensors_raw_data;
    const std::vector<TensorInfo>*                     _tensors_info     = nullptr;
          DepthCalculationInterface*                   _depth_calculation_interface = nullptr;
};


#ifdef HOST_PYTHON_MODULE
struct PyTensorEntryContainerIterator
{
    PyTensorEntryContainerIterator(
        TensorEntryContainer &seq,
        py::object ref
    )
        : seq(seq)
        , ref(ref)
    {}

    std::vector<TensorEntry> next()
    {
        if (index == seq.size())
        {
            throw py::stop_iteration();
        }

        return seq.getByIndex(index++);
    }

    TensorEntryContainer &seq;
    py::object ref;
    size_t index = 0;
};
#endif
