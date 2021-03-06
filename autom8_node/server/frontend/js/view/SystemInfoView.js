 (function() {
  var View = autom8.mvc.View;
  var EMPTY_PASSWORD = '••••••••';

  var redraw = function() {
    var model = this.systemModel;

    this.clearChildren();

    var view = View.fromTemplateId(
      'autom8-View-SystemInfoTable', {
        controller: model.get('system_description') || model.get('system_id'),
        fingerprint: model.get('fingerprint'),
        port: model.get('port'),
        webClientPort: model.get('webClientPort'),
        nativeVersion: model.get('version')
      }
    );

    this.addChild(view, {prependToElement: this.$el.find('.content')});

    setTimeout(function() {
      var connected = autom8.client.connected;
      var $el = this.$('.connection');
      $el.toggleClass('connected', connected);
    }.bind(this));

    this.$('.password-input').val(EMPTY_PASSWORD);
    this.enable(!model.get('running'));

    var systemList = this.systemModel.get('systemList');
    var selectedSystem = this.systemModel.get('system_id');
    $('#system-selection-dropdown').empty().append(
      createDropdown.call(this, systemList, selectedSystem)
    );
  };

  var createDropdown = function(systemList, selectedSystem) {
    var $el = $('<ul class="dropdown-menu"></div>');

    var name, cls;
    if (systemList && systemList.length) {
      for (var i = 0; i < systemList.length; i++) {
        name = systemList.at(i).get('name');
        cls = (selectedSystem === name) ? 'system selected' : 'system';

        $el.append($(
          '<li class="' + cls + '" data-system-id="' + name + '""><label>' + name + '</label></li>'
        ));
      }
    }

    return $el;
  };

  var SystemInfoView = View.extend({
    template: 'autom8-View-SystemInfo',

    events: {
      'input .password-input': function(event) {
        this.passwordChanged = true;

        if (event.keyCode === 13) {
          $(event.target).blur();
        }
      },

      'focus .password-input': function(event) {
        var $input = $(event.target);
        if ($input.val() === EMPTY_PASSWORD) {
          $input.val('');
        }
      },

      'blur .password-input': function(event) {
        var $input = $(event.target);
        if ($input.val() === '') {
          $input.val(EMPTY_PASSWORD);
          this.passwordChanged = false;
        }
      },

      'change .port-input': function(event) {
        this.portChanged = true;
      },

      'change .web-client-port-input': function(event) {
        this.webClientPortChanged = true;
      },

      'touch .system': function(event) {
        this.$('#system-selection-dropdown').dropdown('hide');
        var system = $(event.currentTarget).attr('data-system-id');
        if (system) {
          this.trigger('system:selected', system);
        }
      }
    },

    onCreate: function(options) {
      this.systemModel = autom8.model.SystemModel;
      this.systemModel.on('change', redraw, this);

      $('.main-content-left').on('scroll', function() {
        this.$('#system-selection-dropdown').dropdown('hide');
      }.bind(this));

      this.render();
    },

    onDestroy: function() {
      this.systemModel.off('change', redraw, this);
    },

    render: function() {
      redraw.call(this);
    },

    enable: function(enabled) {
      enabled = enabled || (enabled === undefined);

      this.$('.content').toggleClass('disabled', !enabled);
      $inputs = this.$('input');

      if (enabled) {
        $inputs.removeAttr("disabled");
      }
      else {
        $inputs.attr("disabled", "disabled");
      }
    },

    dirty: function() {
      return this.passwordChanged || this.webClientPortChanged || this.portChanged;
    },

    resetDirtyState: function() {
      this.$('.password-input').val(EMPTY_PASSWORD);
      this.passwordChanged = this.portChanged = this.webClientPortChanged = false;
    }
  });

  namespace("autom8.view").SystemInfoView = SystemInfoView;
}());
