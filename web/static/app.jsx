// A pedal which is available to be added an as active pedal.
class AvailablePedal extends React.Component {
  constructor(props) {
    super(props);
  }

  // Add this pedal to the active pedal list.
  add() {
    $.get("/add_pedal/" + this.props.name)
      .done(this.props.onChange);
  }

  render() {
    return (
      <button class="btn btn-primary btn-small"
              onClick={this.add.bind(this)}>
        {this.props.name}
      </button>
    )
  }
}

// The list of pedals available to be added to the board.
class AvailablePedalList extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      "pedals": []
    };
  }

  // Fetch the list of available pedals from the server.
  componentDidMount() {
    $.get("/available_pedals")
      .done((pedals) => {
        this.setState({
          "pedals": pedals,
        });
      });
  }

  render() {
    const availablePedals = this.state.pedals.map((pedal) => {
      return (<AvailablePedal
                 name={pedal}
                 onChange={this.props.onChange} />)
    });

    return (
      <div class="available-pedal-list" align="center">
        {availablePedals}
      </div>
    )
  }
}

// A knob which adjusts the settings of an active pedal.
class Knob extends React.Component {
  constructor(props) {
    super(props);
  }

  tweak(sign) {
    const knobUpdate = {
      'name': this.props.name,
      'value': this.props.value + (sign * this.props.tweakAmount),
    };
    $.get('/adjust_knob/' + this.props.pedalIndex, knobUpdate)
      .done(this.props.refresh);
  }

  // Send the knob update to the server and refresh the board.
  onChange(event) {
    const knobUpdate = {
      'name': this.props.name,
      'value': event.target.value,
    };

    $.get('/adjust_knob/' + this.props.pedalIndex, knobUpdate)
      .done(this.props.refresh);
  }

  render() {
    return (
      <div class="knob">
        {this.props.name}

        <button class="btn btn-primary btn-small"
                onClick={this.tweak.bind(this, -1)}>
          -
        </button>
        <input value={this.props.value} onChange={this.onChange.bind(this)} />
        <button class="btn btn-primary btn-small"
                onClick={this.tweak.bind(this, 1)}>
          +
        </button>
      </div>
    )
  }
}

// An active pedal which is running on the board and applying effects to the
// signal.
class ActivePedal extends React.Component {
  constructor(props) {
    super(props);
  }

  // Remove this pedal from the board.
  remove() {
    $.get('/remove_pedal/' + this.props.index)
     .done(this.props.refresh);
  }

  render() {
    const knobs = this.props.knobs.map((knob) => {
      return (
          <Knob
            name={knob.name}
            value={knob.value}
            tweakAmount={knob.tweak_amount}
            pedalIndex={this.props.index}
            refresh={this.props.refresh} />
      )
    });

    return (
      <div class="active-pedal">
        <h4 class="card-title">
          {this.props.name}
        </h4>

        {knobs}

        <button
          class="btn btn-danger btn-small"
          onClick={this.remove.bind(this)} >
          Remove
        </button>
      </div>
    )
  }
}

// The board of active pedals.
class PedalBoard extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      'pedals': []
    };

    // Register a refresh function with the app so that other components can
    // trigger a board refresh when they change state.
    this.props.registerRefresh(this.refresh.bind(this));
  }

  componentDidMount() {
    this.refresh();
  }

  // Refetch the active pedals from the server.
  refresh() {
    $.get('/active_pedals').done(response => {
      this.setState({
        'pedals': response.pedals
      });
    });
  }

  render() {
    return this.state.pedals.map((pedal, index) => {
      return (
          <div className="row">
            <div className="col-md-8 offset-md-2 col-xs-12">
              <div className="card">
                <div className="card-block">
                  <ActivePedal
                     name={pedal.name}
                     knobs={pedal.knobs}
                     index={index}
                     refresh={this.refresh.bind(this)} />
                </div>
              </div>
            </div>
          </div>
      )
    });
  }
}

class App extends React.Component {
  constructor(props) {
    super(props);

    this.refreshHandlers = [];
  }

  registerRefreshHandler(handler) {
    this.refreshHandlers.push(handler);
  }

  refresh() {
    for (const refreshHandler of this.refreshHandlers) {
      refreshHandler();
    }
  }

  render() {
    return (
      <div>
        <AvailablePedalList onChange={this.refresh.bind(this)} />
        <hr />
        <PedalBoard
           registerRefresh={this.registerRefreshHandler.bind(this)} />
      </div>
    )
  }
}

// Fetch the feed from the API and render it.
$(document).ready(() => {
  ReactDOM.render(
      <App />,
      document.getElementById('pedalboard')
  );
});

