namespace("autom8.controller").MainController = (function() {
  function refreshStatus() {
    autom8.model.SystemModel.fetch();
  }

  function reconnect() {
    var c = autom8.client;
    var s = autom8.client.getState();
    if (!c.connected && !c.connecting && s !== "authenticating") {
      if (autom8.client.getState() !== "expired") {
        autom8.client.connect({silent: true});
      }
    }
  }

  function onStartClicked() {
    this.view.serverControlView.$el.addClass('updating');

    this.systemInfoController.save()

    .then(function() {
      return autom8.client.rpc.send({
        component: "server", command: "start", options: { }
      });
    })

    .done(refreshStatus);
  }

  function onStopClicked() {
    this.view.serverControlView.$el.addClass('updating');

    autom8.client.rpc.send({
      component: "server", command: "stop", options: { }
    })

    .done(refreshStatus);
  }

  return autom8.mvc.Controller.extend({
    mixins: [
      autom8.mvc.mixins.ControllerContainer
    ],

    onCreate: function(options) {
      this.view = new autom8.view.MainView({ el: $('.main-content-left') });

      this.messagingController = this.addChild(
        new autom8.controller.ConnectionMessagingController()
      );

      this.headerController = this.addChild(
        new autom8.controller.HeaderController({
          el: $('.main-header')
        })
      );

      this.signInController = this.addChild(
        new autom8.controller.SignInController({
          el: $('.sign-in')
        }),
        {
          resume: false
        }
      );

      this.systemInfoController = this.addChild(
        new autom8.controller.SystemInfoController()
      );

      this.deviceListController = this.addChild(
        new autom8.controller.DeviceListController()
      );

      this.view.serverControlView.on('start:clicked', onStartClicked, this);
      this.view.serverControlView.on('stop:clicked', onStopClicked, this);

      autom8.client.on('connected', this.onConnected, this);
      autom8.client.on('disconnected', this.onDisconnected, this);
      autom8.client.on('expired', this.onDisconnected, this);
      autom8.client.on('state:changed', this.onClientStateChanged, this);
      autom8.client.on('resync', this.onResync, this);

      this.showSignIn();
      reconnect();
    },

    showSignIn: function() {
      this.view.$el.addClass('show-sign-in');
      this.signInController.resume();
    },

    showDevices: function() {
      this.view.$el.removeClass('show-sign-in');
      this.signInController.pause();
    },

    onDestroy: function() {
      autom8.client.off('connected', this.onConnected, this);
      autom8.client.off('disconnected', this.onDisconnected, this);
      autom8.client.off('expired', this.onDisconnected, this);
      autom8.client.off('state:changed', this.onClientStateChanged, this);
      autom8.client.off('resync', this.onResync, this);
    },

    onConnected: function() {
      this.showDevices();
      refreshStatus();
    },

    onDisconnected: function() {
      if (autom8.client.getState() === "expired") {
        this.messagingController.closeDisconnectedDialog();
        this.showSignIn();
      }
    },

    onClientStateChanged: function(state) {
      if (state === "authenticated") {
        autom8.client.connect();
      }
    },

    onResync: function() {
      refreshStatus();
    }
  });
}());