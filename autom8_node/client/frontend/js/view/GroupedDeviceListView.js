namespace("autom8.view").GroupedDeviceListView = (function() {
  var View = autom8.mvc.View;

  function groupComparator(a, b) {
    return a.sortKey() > b.sortKey() ? 1 : -1;
  }

  function createGroupedDeviceList(deviceList) {
      /* build map of groups */
      var groupMap = { };
      deviceList.each(function(device) {
        _.each(device.get('groups'), function(group) {
          groupMap[group] = groupMap[group] || [];
          groupMap[group].push(device);
        });
      });

      /* sorting function for group of subitems */
      var listOptions = {
        comparator: deviceList.comparator
      };

      /* flatten to array so we can sort */
      var groupedDeviceList = [];
      _.each(groupMap, function(value, key) {
        if (value && key) {
          var groupModel = new autom8.model.DeviceGroup({
            name: key,
            deviceList: new autom8.model.DeviceList(value, listOptions)
          });

          groupedDeviceList.push(groupModel);
        }
      });

      /* sort */
      groupedDeviceList.sort(groupComparator);

      return groupedDeviceList;
  }

  var _super_ = autom8.view.DeviceListView;

  return _super_.extend({
    tagName: "ul",

    events: {
      "touch .device-row .expander": function(e) {
        this.onToggleCollapsed($(e.currentTarget));
      },

      "touch .device-row": function(e) {
        this.onDeviceRowTouched($(e.currentTarget));
      },

      "touch .device-row .extras": function(e) {
          var device = this.deviceFromElement($(e.currentTarget));
          this.trigger('extras:clicked', device);
      }
    },

    onCreate: function(options) {
      _super_.prototype.onCreate.call(this, options);
      this.groupedDeviceList = [];
    },

    onDeviceRowTouched: function($el) {
      var groupIndex = $el.attr("data-group");
      var itemIndex = $el.attr("data-index");

      if (groupIndex && itemIndex) {
        groupIndex = parseInt(groupIndex, 10);
        itemIndex = parseInt(itemIndex, 10);
        var device = this.groupedDeviceList[groupIndex].deviceList().at(itemIndex);
        this.trigger('devicerow:clicked', device);
      }
      else if (groupIndex) {
        groupIndex = parseInt(groupIndex, 10);
        var group = this.groupedDeviceList[groupIndex];
        this.trigger('grouprow:clicked', group);
      }
    },

    onToggleCollapsed: function($target) {
      var $root = $target.parents('.device-group-container');
      var $group = $root.find('.device-row.group');

      var groupIndex = $group.attr("data-group");
      if (groupIndex) {
        this.listView.views[Number(groupIndex)].toggleCollapsed();
      }
    },

    onResume: function() {
      if (this.dirty) {
        this.dirty = false;
        this.render();
      }
    },

    onRender: function() {
      if (this.paused) {
        this.dirty = true;
        return;
      }

      this.listView.clearChildren();

      if (this.groupedDeviceList.length < 1) {
        return;
      }

      var self = this;
      _.each(this.groupedDeviceList, function(group, index) {
        var options = {
          attrs: {
            group: index
          }
        };

        var groupRow = autom8.view.DeviceRowFactory.create(group, options);
        self.listView.addChild(groupRow);
      });
    },

    setDeviceList: function(deviceList) {
      if (deviceList === this.deviceList) {
        return;
      }

      this.deviceList = deviceList;
      this.groupedDeviceList = createGroupedDeviceList(deviceList);
      this.render();
    },

    resort: function() {
      if (this.deviceList) {
        _.each(this.groupedDeviceList, function(group) {
          group.deviceList().sort();
        });

        this.groupedDeviceList.sort(groupComparator);
        this.render();
      }
    },

    deviceFromElement: function($el) {
      $el = $el.parents('.device-row');

      if ($el) {
        var groupIndex = $el.attr("data-group");
        var itemIndex = $el.attr("data-index");

        if (groupIndex && itemIndex) {
          groupIndex = Number(groupIndex);
          itemIndex = Number(itemIndex);
          return this.groupedDeviceList[groupIndex].deviceList().at(itemIndex);
        }
      }

      return null;
    }
  });
}());
