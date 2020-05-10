class AvailablePedal extends React.Component {
  constructor(props) {
    super(props);
  }

  add() {
    $.get("/add_pedal/" + this.props.name, () => {
      this.props.onChange();
    });
  }

  render() {
    return (
      <p>
        {this.props.name} <button class="btn btn-primary btn-small"
                                  onClick={this.add.bind(this)}>Add</button>
      </p>
    )
  }
}

class PedalList extends React.Component {
  constructor(props) {
    super(props);
    this.state = {"pedals": []};
  }

  componentDidMount() {
    $.get("/available_pedals").done((pedals) => {
      this.setState({"pedals": pedals});
    });
  }

  render() {
    return this.state.pedals.map((pedal) => {
      return (<AvailablePedal name={pedal} onChange={this.props.onChange} />)
    });
  }
}

class Knob extends React.Component {
  constructor(props) {
    super(props);
  }

  tweak(event) {
    console.log("Tweaking knob " + this.props.name);
    console.log("Value is " + event.target.value);
  }

  onChange(event) {
    const knobUpdate = {
      'name': this.props.name,
      'value': event.target.value,
    };
    $.get('/adjust_knob/' + this.props.pedalIndex, knobUpdate).done(this.props.refresh);
  }

  render() {
    const boundTweak = this.tweak.bind(this);
    return (
      <p>
        {this.props.name}
        <button class="btn btn-primary btn-small" onClick={boundTweak}>
          +
        </button>
        <input value={this.props.value} onChange={this.onChange.bind(this)} />
        <button class="btn btn-primary btn-small" onClick={boundTweak}>
          -
        </button>
      </p>
    )
  }
}

class ActivePedal extends React.Component {
  constructor(props) {
    super(props);
  }

  render() {
    const knobs = this.props.knobs.map((knob) => {
      return (<Knob name={knob.name} value={knob.value} pedalIndex={this.props.index} refresh={this.props.refresh} />)
    });
    return (
      <p>
        {this.props.name}
        {knobs}
      </p>
    )
  }
}

class PedalBoard extends React.Component {
  constructor(props) {
    super(props);
    this.state = {'pedals': []};
    this.props.registerRefresh(() => this.refresh());
  }

  componentDidMount() {
    this.refresh();
  }

  refresh() {
    $.get('/active_pedals').done(pedals => {
      this.setState({'pedals': pedals});
    });
  }

  render() {
    return this.state.pedals.map((pedal, index) => {
      return <ActivePedal name={pedal.name} knobs={pedal.knobs} index={index} refresh={this.refresh.bind(this)} />
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
        <PedalList onChange={this.refresh.bind(this)} />
        <hr />
        <PedalBoard registerRefresh={this.registerRefreshHandler.bind(this)} />
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

