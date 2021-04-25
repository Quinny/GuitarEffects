// A pedal which is available to be added an as active pedal.
class AvailablePedal extends React.Component {
  constructor(props) {
    super(props);
  }

  // Add this pedal to the active pedal list.
  add() {
    $.get("/add_pedal/" + this.props.name);
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
      "pedals": [],
      "temp": 0
    };
  }

  // Fetch the list of available pedals from the server.
  componentDidMount() {
    $.get("/available_pedals")
      .done((pedals) => {
        this.setState({
          "pedals": pedals.sort(),
        });
      });


    this.queryTemp();
    setInterval(this.queryTemp.bind(this), 10000);
  }

  // Query the core temperature of the device.
  queryTemp() {
    $.get("/temp").done(response => {
      this.setState({'temp': response.temp});
    });
  }


  render() {
    const availablePedals = this.state.pedals.map((pedal) => {
      return (<AvailablePedal name={pedal} />)
    });

    return (
      <div class="available-pedal-list" align="center">
        {availablePedals}
        <button class="btn btn-secondary btn-small">
          {this.state.temp} °C
        </button>
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
    $.get('/adjust_knob/' + this.props.pedalIndex, knobUpdate);
  }

  // Send the knob update to the server.
  onChange(event) {
    const knobUpdate = {
      'name': this.props.name,
      'value': event.target.value,
    };

    $.get('/adjust_knob/' + this.props.pedalIndex, knobUpdate);
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
    $.get('/remove_pedal/' + this.props.index);
  }

  // Push the button on this pedal.
  push() {
    $.get('/push_button/' + this.props.index);
  }

  render() {
    const knobs = this.props.knobs.map((knob) => {
      return (
          <Knob
            name={knob.name}
            value={knob.value}
            tweakAmount={knob.tweak_amount}
            pedalIndex={this.props.index} />
      )
    });

    return (
      <div class="active-pedal">
        <h4 class="card-title">
          {this.props.name}
          {this.props.summary &&
            <h6 className="d-inline-block card-subtitle text-muted float-right margin-top">
              #{this.props.index}
            </h6>}
        </h4>

        {!this.props.summary && knobs}
        <br />
        <button
          class="btn btn-small btn-secondary"
          onClick={this.push.bind(this)}>
          {this.props.state}
        </button>
        &nbsp;

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
  }

  componentDidMount() {
    var sock = new WebSocket("ws://" + window.location.host + "/updates");
    sock.onmessage = e => {
      this.refresh();
    }
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
    var fullView = this.state.pedals.map((pedal, index) => {
      return (
          <div className="row">
            <div className="col-md-8 offset-md-2 col-xs-12">
              <div className="card">
                <div className="card-block">
                  <ActivePedal
                     name={pedal.name}
                     state={pedal.state}
                     knobs={pedal.knobs}
                     index={index}
                     summary={false} />
                </div>
              </div>
            </div>
          </div>
      )
    });

    var summaryView = this.state.pedals.map((pedal, index) => {
      return (
          <div className="col-xs-2">
            <div className="card">
              <div className="card-block">
                <ActivePedal
                   name={pedal.name}
                   state={pedal.state}
                   knobs={pedal.knobs}
                   index={index}
                   summary={true} />
              </div>
            </div>
          </div>
      )
    });

    return (
      <div>
        <div className="row justify-content-md-center">
          {summaryView}
        </div>
        {fullView}
      </div>
    );
  }
}

class App extends React.Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (
      <div>
        <AvailablePedalList />
        <hr />
        <PedalBoard />
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

